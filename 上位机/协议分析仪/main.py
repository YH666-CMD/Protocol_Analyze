# -*- coding: utf-8 -*-
"""协议分析仪上位机。窗口大约和串口助手一样大。平滑和电压换算只在这里做。"""

import os
import queue
import struct
import sys
import threading
import tkinter as tk
from tkinter import filedialog, scrolledtext, ttk

try:
    import serial.tools.list_ports as list_ports
except ImportError:
    list_ports = None

from logutil import ChannelLog, app_dir
from ota_pkg import load_cipher_faces, send_upgrade
from process.scale import adc_volts, pwm_real
from process.smooth import ema_step, linear_resample
from protocol import (
    BAUD_CODE, CMD_ADC, CMD_CTRL, CMD_I2C, CMD_I2C_CFG, CMD_INFO, CMD_NACK,
    CMD_PWM_IN, CMD_PWM_OUT, CMD_SPI, CMD_SPI_CFG, CMD_UART_LINE, CMD_UART_RX,
    CMD_UART_TX, MOD_ADC, MOD_I2C, MOD_PWM, MOD_SPI, MOD_UART, OP_OFF, OP_ON,
    RES_NAME, VID_CDC, VID_CH340, Link, hex_line,
)

HERE = app_dir()
LOG_ROOT = os.path.join(HERE, "日志")
KEEP = 240
COLORS = ("#1f77b4", "#d62728", "#2ca02c")


def key_path():
    candidates = [
        os.path.join(HERE, "密钥.txt"),
        os.path.normpath(os.path.join(HERE, "..", "..", "工具", "加密工具", "密钥.txt")),
        os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "工具", "加密工具", "密钥.txt")),
    ]
    for path in candidates:
        if os.path.isfile(path):
            return path
    return candidates[1]


class Host(object):
    def __init__(self, root):
        self.root = root
        self.root.title("协议分析仪")
        self.root.geometry("780x500")
        self.root.minsize(720, 460)
        self.busy = False
        self.ui = queue.Queue()
        self.ports = []
        self.logs = {}
        for name in ("串口", "ADC", "SPI", "I2C", "PWM", "升级"):
            self.logs[name] = ChannelLog(os.path.join(LOG_ROOT, name), name)
        self.link = Link(lambda text: self.note("升级", text))
        self.ema = [None, None, None]
        self.raw = [[], [], []]
        self.smooth = [[], [], []]
        self.show_volt = tk.BooleanVar(value=True)
        self.alpha = tk.DoubleVar(value=0.25)
        self.hex_mode = tk.BooleanVar(value=True)

        top = ttk.Frame(root, padding=6)
        top.pack(fill=tk.X)
        ttk.Label(top, text="USB").pack(side=tk.LEFT)
        self.port_box = ttk.Combobox(top, width=46, state="readonly")
        self.port_box.pack(side=tk.LEFT, padx=4)
        ttk.Button(top, text="刷新", command=self.refresh).pack(side=tk.LEFT)
        self.conn_btn = ttk.Button(top, text="连接", command=self.toggle)
        self.conn_btn.pack(side=tk.LEFT, padx=4)
        self.status = ttk.Label(top, text="未连接")
        self.status.pack(side=tk.LEFT, padx=8)

        self.book = ttk.Notebook(root)
        self.book.pack(fill=tk.BOTH, expand=True, padx=6, pady=4)
        self.pages = []
        self._build_uart()
        self._build_adc()
        self._build_spi()
        self._build_i2c()
        self._build_pwm()
        self._build_ota()
        self.book.bind("<<NotebookTabChanged>>", self.on_tab)
        self.note("串口", "日志目录：%s" % LOG_ROOT)
        self.note("串口", "请连接 1A86:FE0C。切页会要求板子关掉其它功能；板端还没接开关时会拒绝，串口收发和升级仍然可用。")
        self.refresh()
        self.root.after(80, self.pump)
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def note(self, channel, text):
        line = self.logs[channel].write(text)
        self.ui.put((channel, line))

    def _view(self, parent, height=8):
        box = scrolledtext.ScrolledText(parent, height=height, font=("Consolas", 9))
        box.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)
        return box

    def _build_uart(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="串口")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        ttk.Label(bar, text="波特率").pack(side=tk.LEFT)
        self.baud = ttk.Combobox(bar, width=10, state="readonly", values=list(BAUD_CODE.keys()))
        self.baud.set("115200")
        self.baud.pack(side=tk.LEFT, padx=4)
        ttk.Button(bar, text="设置", command=self.set_baud).pack(side=tk.LEFT)
        ttk.Checkbutton(bar, text="十六进制", variable=self.hex_mode).pack(side=tk.LEFT, padx=8)
        self.uart_send = ttk.Entry(bar)
        self.uart_send.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=4)
        ttk.Button(bar, text="发送", command=self.send_uart).pack(side=tk.LEFT)
        self.view_uart = self._view(page, 16)

    def _build_adc(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="ADC")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        ttk.Button(bar, text="开始", command=lambda: self.module_cmd(MOD_ADC, OP_ON, "ADC")).pack(side=tk.LEFT)
        ttk.Button(bar, text="停止", command=lambda: self.module_cmd(MOD_ADC, OP_OFF, "ADC")).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(bar, text="显示伏特（×11）", variable=self.show_volt).pack(side=tk.LEFT, padx=8)
        ttk.Label(bar, text="EMA").pack(side=tk.LEFT)
        ttk.Scale(bar, from_=0.05, to=1.0, variable=self.alpha, length=120).pack(side=tk.LEFT, padx=4)
        self.canvas = tk.Canvas(page, height=220, bg="white", highlightthickness=1)
        self.canvas.pack(fill=tk.X, padx=4)
        self.view_adc = self._view(page, 6)

    def _build_spi(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="SPI")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        ttk.Label(bar, text="模式").pack(side=tk.LEFT)
        self.spi_mode = ttk.Combobox(bar, width=4, state="readonly", values=["0", "1", "2", "3"])
        self.spi_mode.set("0")
        self.spi_mode.pack(side=tk.LEFT, padx=4)
        ttk.Button(bar, text="切换模式", command=self.spi_cfg).pack(side=tk.LEFT)
        self.spi_send = ttk.Entry(bar)
        self.spi_send.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=4)
        ttk.Button(bar, text="发送", command=self.spi_tx).pack(side=tk.LEFT)
        self.view_spi = self._view(page, 16)

    def _build_i2c(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="I2C")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        self.i2c_role = ttk.Combobox(bar, width=6, state="readonly", values=["主机", "从机"])
        self.i2c_role.set("主机")
        self.i2c_role.pack(side=tk.LEFT)
        ttk.Label(bar, text="地址").pack(side=tk.LEFT, padx=4)
        self.i2c_addr = ttk.Entry(bar, width=8)
        self.i2c_addr.insert(0, "50")
        self.i2c_addr.pack(side=tk.LEFT)
        ttk.Button(bar, text="切换", command=self.i2c_cfg).pack(side=tk.LEFT, padx=4)
        ttk.Button(bar, text="扫描地址", command=self.i2c_scan).pack(side=tk.LEFT)
        self.i2c_send = ttk.Entry(bar)
        self.i2c_send.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=4)
        ttk.Button(bar, text="发送", command=self.i2c_tx).pack(side=tk.LEFT)
        self.view_i2c = self._view(page, 16)

    def _build_pwm(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="PWM")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        ttk.Label(bar, text="频率Hz").pack(side=tk.LEFT)
        self.pwm_hz = ttk.Entry(bar, width=8)
        self.pwm_hz.insert(0, "1000")
        self.pwm_hz.pack(side=tk.LEFT, padx=4)
        ttk.Label(bar, text="占空%").pack(side=tk.LEFT)
        self.pwm_duty = ttk.Entry(bar, width=6)
        self.pwm_duty.insert(0, "50")
        self.pwm_duty.pack(side=tk.LEFT, padx=4)
        ttk.Button(bar, text="输出", command=self.pwm_out).pack(side=tk.LEFT)
        ttk.Button(bar, text="停止", command=lambda: self.module_cmd(MOD_PWM, OP_OFF, "PWM")).pack(side=tk.LEFT, padx=4)
        self.view_pwm = self._view(page, 16)

    def _build_ota(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="升级")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        self.hex_path = tk.StringVar()
        ttk.Entry(bar, textvariable=self.hex_path).pack(side=tk.LEFT, fill=tk.X, expand=True)
        ttk.Button(bar, text="选文件", command=self.pick_hex).pack(side=tk.LEFT, padx=4)
        ttk.Button(bar, text="查询", command=lambda: self.run_bg(self.do_info)).pack(side=tk.LEFT)
        ttk.Button(bar, text="升级", command=lambda: self.run_bg(self.do_upgrade)).pack(side=tk.LEFT, padx=4)
        self.view_ota = self._view(page, 16)
        self.note("升级", "选择加密.hex。不要点确认，新面自己留下。密钥：%s" % key_path())

    def refresh(self):
        self.ports = []
        labels = []
        if list_ports is None:
            self.note("串口", "没有 pyserial。先执行：py -3 -m pip install pyserial")
            return
        for item in list_ports.comports():
            mark = ""
            ident = (item.vid, item.pid)
            if ident == VID_CDC:
                mark = "  [设备]"
            elif ident == VID_CH340 or "CH340" in (item.description or "").upper():
                mark = "  [CH340]"
            labels.append("%s  %s%s" % (item.device, item.description, mark))
            self.ports.append(item)
        self.port_box["values"] = labels
        if labels:
            self.port_box.current(0)
            for index, item in enumerate(self.ports):
                if (item.vid, item.pid) == VID_CDC:
                    self.port_box.current(index)
                    break

    def selected(self):
        index = self.port_box.current()
        if index < 0 or index >= len(self.ports):
            return None
        return self.ports[index]

    def toggle(self):
        if self.link.ser is not None:
            self.link.close()
            self.conn_btn["text"] = "连接"
            self.status["text"] = "未连接"
            self.note("串口", "已断开")
            return
        item = self.selected()
        if item is None:
            self.note("串口", "没有选中端口")
            return
        if (item.vid, item.pid) == VID_CH340 or "CH340" in (item.description or "").upper():
            self.note("串口", "这是 CH340，协议帧到不了芯片。请选带 [设备] 的口。")
            return
        try:
            self.link.open(item.device)
        except Exception as exc:
            self.note("串口", "打开失败：%s" % exc)
            return
        self.conn_btn["text"] = "断开"
        self.status["text"] = item.device
        self.note("串口", "已连接 %s" % item.device)
        self.on_tab()

    def on_tab(self, _event=None):
        if self.link.ser is None:
            return
        index = self.book.index(self.book.select())
        names = ["串口", "ADC", "SPI", "I2C", "PWM", "升级"]
        mods = [MOD_UART, MOD_ADC, MOD_SPI, MOD_I2C, MOD_PWM, None]
        current = names[index]

        def work():
            for i, mod in enumerate(mods):
                if mod is None or i == index:
                    continue
                self.ctrl(mod, OP_OFF, names[i])
            if mods[index] is not None:
                self.ctrl(mods[index], OP_ON, current)
            self.note(current, "已切到本页，其它功能已要求关闭")

        threading.Thread(target=work, daemon=True).start()

    def ctrl(self, mod, op, name):
        kind, body = self.link.transact(CMD_CTRL, bytes([mod, op]), 0.4)
        if kind is None:
            self.note(name, "%s 开关没有应答" % name)
            return
        if kind == CMD_NACK:
            code = body[1] if len(body) > 1 else -1
            self.note(name, "%s 开关被拒绝：%s（板端这步还没接上时会这样）" % (name, RES_NAME.get(code, str(code))))

    def module_cmd(self, mod, op, name):
        if self.link.ser is None:
            self.note(name, "还没有连接")
            return
        threading.Thread(target=lambda: self.ctrl(mod, op, name), daemon=True).start()

    def run_bg(self, func):
        if self.busy:
            self.note("升级", "上一步还没结束")
            return
        if self.link.ser is None:
            self.note("升级", "还没有连接")
            return
        self.busy = True

        def wrap():
            try:
                func()
            except Exception as exc:
                self.note("升级", "中断：%s" % exc)
            self.busy = False

        threading.Thread(target=wrap, daemon=True).start()

    def need_ack(self, cmd, data, timeout, step):
        kind, body = self.link.transact(cmd, data, timeout)
        if kind is None:
            self.note("升级", "%s失败：没有回复" % step)
            return None
        if kind == CMD_NACK:
            code = body[1] if len(body) > 1 else -1
            self.note("升级", "%s被拒绝：%s" % (step, RES_NAME.get(code, str(code))))
            return None
        self.note("升级", "%s成功" % step)
        return body

    def set_baud(self):
        code = BAUD_CODE.get(self.baud.get())
        if code is None or self.link.ser is None:
            return
        threading.Thread(target=lambda: self.link.transact(CMD_UART_LINE, bytes([code]), 0.6), daemon=True).start()
        self.note("串口", "请求波特率 %s" % self.baud.get())

    def send_uart(self):
        text = self.uart_send.get().strip()
        if not text or self.link.ser is None:
            return
        try:
            if self.hex_mode.get():
                payload = bytes.fromhex(text.replace(" ", ""))
            else:
                payload = text.encode("utf-8")
        except ValueError:
            self.note("串口", "十六进制格式不对")
            return

        def work():
            for off in range(0, len(payload), 48):
                chunk = payload[off:off + 48]
                kind, body = self.link.transact(CMD_UART_TX, chunk, 0.25, want_reply=False)
                if kind == CMD_NACK:
                    code = body[1] if len(body) > 1 else -1
                    self.note("串口", "发送被拒绝：%s" % RES_NAME.get(code, str(code)))
                    return
            self.note("串口", "发出 %s" % hex_line(payload))

        threading.Thread(target=work, daemon=True).start()

    def spi_cfg(self):
        mode = int(self.spi_mode.get())
        self.note("SPI", "请求模式 %u" % mode)
        self.module_send(CMD_SPI_CFG, bytes([mode, 0]), "SPI")

    def spi_tx(self):
        raw = self._hex_bytes(self.spi_send.get(), "SPI")
        if raw is None:
            return
        self.note("SPI", "请求发送 %s" % hex_line(raw))
        self.module_send(CMD_SPI_CFG, bytes([int(self.spi_mode.get()), 1]) + raw[:40], "SPI")

    def i2c_cfg(self):
        role = 0 if self.i2c_role.get() == "主机" else 1
        addr = self._addr(self.i2c_addr.get())
        self.note("I2C", "请求%s，地址 0x%02X" % (self.i2c_role.get(), addr))
        self.module_send(CMD_I2C_CFG, bytes([role, addr, 0]), "I2C")

    def i2c_scan(self):
        self.note("I2C", "请求扫描 7 位地址")
        self.module_send(CMD_I2C_CFG, bytes([0, 0, 1]), "I2C")

    def i2c_tx(self):
        raw = self._hex_bytes(self.i2c_send.get(), "I2C")
        if raw is None:
            return
        role = 0 if self.i2c_role.get() == "主机" else 1
        addr = self._addr(self.i2c_addr.get())
        self.note("I2C", "请求发送 %s" % hex_line(raw))
        self.module_send(CMD_I2C_CFG, bytes([role, addr, 2]) + raw[:40], "I2C")

    def pwm_out(self):
        try:
            hz = int(self.pwm_hz.get())
            duty = int(self.pwm_duty.get())
        except ValueError:
            self.note("PWM", "频率和占空比要是整数")
            return
        self.note("PWM", "请求输出 %u Hz，占空 %u%%" % (hz, duty))
        self.module_send(CMD_PWM_OUT, struct.pack("<HH", hz & 0xFFFF, duty & 0xFFFF), "PWM")

    def module_send(self, cmd, data, name):
        if self.link.ser is None:
            self.note(name, "还没有连接")
            return

        def work():
            kind, body = self.link.transact(cmd, data, 0.6)
            if kind == CMD_NACK:
                code = body[1] if len(body) > 1 else -1
                self.note(name, "板端拒绝：%s" % RES_NAME.get(code, str(code)))
            elif kind is None:
                self.note(name, "没有应答")

        threading.Thread(target=work, daemon=True).start()

    def _hex_bytes(self, text, name):
        text = text.strip()
        if not text:
            self.note(name, "没有要发的数据")
            return None
        try:
            return bytes.fromhex(text.replace(" ", ""))
        except ValueError:
            self.note(name, "十六进制格式不对")
            return None

    def _addr(self, text):
        try:
            return int(text, 16) & 0x7F
        except ValueError:
            return 0

    def pick_hex(self):
        path = filedialog.askopenfilename(filetypes=[("hex", "*.hex"), ("全部", "*.*")])
        if path:
            self.hex_path.set(path)
            self.note("升级", "已选 %s" % path)

    def do_info(self):
        self.note("升级", "—— 查询 ——")
        kind, body = self.link.transact(CMD_INFO, b"", 1.0)
        if kind is None:
            self.note("升级", "查询没有回复")
            return
        if kind == CMD_NACK:
            code = body[1] if len(body) > 1 else -1
            self.note("升级", "查询被拒绝：%s" % RES_NAME.get(code, str(code)))

    def do_upgrade(self):
        path = self.hex_path.get().strip()
        if not path:
            self.note("升级", "还没有选择加密.hex")
            return
        faces = load_cipher_faces(path, key_path(), lambda text: self.note("升级", text))
        if faces is None:
            return
        send_upgrade(self.link, faces, lambda text: self.note("升级", text), self.need_ack)

    def pump(self):
        views = {"串口": self.view_uart, "ADC": self.view_adc, "SPI": self.view_spi,
                 "I2C": self.view_i2c, "PWM": self.view_pwm, "升级": self.view_ota}
        while True:
            try:
                channel, line = self.ui.get_nowait()
            except queue.Empty:
                break
            box = views.get(channel)
            if box is not None:
                box.insert(tk.END, line + "\n")
                box.see(tk.END)
        for frame in self.link.drain_side():
            self.on_frame(frame)
        self.draw_adc()
        if self.link.ser is not None and not self.link.alive:
            self.status["text"] = "连接中断"
        self.root.after(80, self.pump)

    def on_frame(self, frame):
        kind = frame[2]
        body = frame[3:-2]
        if kind == CMD_UART_RX:
            self.note("串口", "收 %s" % hex_line(body))
        elif kind == CMD_ADC:
            self.take_adc(body)
            self.logs["ADC"].write("原始 " + hex_line(body))
        elif kind == CMD_SPI:
            self.note("SPI", "收 %s" % hex_line(body))
        elif kind == CMD_I2C:
            self.note("I2C", "收 %s" % hex_line(body))
        elif kind == CMD_PWM_IN:
            self.take_pwm(body)
        else:
            self.note("升级", "旁路帧 " + hex_line(frame))

    def take_adc(self, body):
        if len(body) < 4 or ((len(body) - 2) % 2) != 0:
            self.note("ADC", "样本长度不对：" + hex_line(body))
            return
        nch = body[1]
        if nch < 1 or nch > 3:
            self.note("ADC", "通道数 %u 不在 1 到 3" % nch)
            return
        words = struct.unpack_from("<%uH" % ((len(body) - 2) // 2), body, 2)
        alpha = float(self.alpha.get())
        for index, sample in enumerate(words):
            ch = index % nch
            self.raw[ch].append(sample)
            self.smooth[ch].append(ema_step(self.ema[ch], sample, alpha))
            self.ema[ch] = self.smooth[ch][-1]
            if len(self.raw[ch]) > KEEP:
                del self.raw[ch][0]
                del self.smooth[ch][0]

    def draw_adc(self):
        canvas = self.canvas
        canvas.delete("all")
        width = max(canvas.winfo_width(), 10)
        height = max(canvas.winfo_height(), 10)
        for ch in range(3):
            series = self.smooth[ch]
            if len(series) < 2:
                continue
            if self.show_volt.get():
                series = [adc_volts(v) for v in series]
                top = 3.3 * 11
            else:
                top = 4095.0
            points = linear_resample(series, min(width, 400))
            coords = []
            for i, value in enumerate(points):
                x = i * (width - 4) / max(1, len(points) - 1) + 2
                y = height - 4 - (value / top) * (height - 8)
                coords.extend((x, y))
            canvas.create_line(*coords, fill=COLORS[ch], width=1)
        canvas.create_text(8, 8, anchor="nw", text="蓝 CH0  红 CH1  绿 CH2", fill="#444")

    def take_pwm(self, body):
        if len(body) < 8:
            self.note("PWM", "输入 " + hex_line(body))
            return
        period, high = struct.unpack_from("<II", body, 0)
        self.note("PWM", "原始周期 %u 高电平 %u；显示 ×11 后 %g / %g" % (
            period, high, pwm_real(period), pwm_real(high)))

    def on_close(self):
        self.link.close()
        for item in self.logs.values():
            item.close()
        self.root.destroy()


def main():
    if sys.platform == "win32":
        try:
            import ctypes
            ctypes.windll.shcore.SetProcessDpiAwareness(1)
        except Exception:
            pass
    root = tk.Tk()
    Host(root)
    root.mainloop()


if __name__ == "__main__":
    main()
