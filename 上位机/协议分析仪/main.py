# -*- coding: utf-8 -*-
"""协议分析仪上位机。平滑和电压换算只在这里做。"""

import os
import queue
import struct
import sys
import threading
import tkinter as tk
from collections import deque
from tkinter import filedialog, scrolledtext, ttk

try:
    import serial.tools.list_ports as list_ports
except ImportError:
    list_ports = None

from adc_scope import AdcScope, sample_us
from logutil import ChannelLog, app_dir
from ota_pkg import load_cipher_faces, send_upgrade
from process.smooth import ema_step
from protocol import (
    BAUD_CODE, CMD_ADC, CMD_CTRL, CMD_I2C, CMD_I2C_CFG, CMD_INFO, CMD_LOG,
    CMD_LOG_RSP, CMD_NACK, CMD_PWM_IN, CMD_PWM_OUT, CMD_SPI, CMD_SPI_CFG,
    CMD_UART_LINE, CMD_UART_RX, CMD_UART_TX, MOD_ADC, MOD_I2C, MOD_PWM, MOD_SPI,
    MOD_UART, OP_OFF, OP_ON, RES_NAME, VID_CDC, VID_CH340, Link, describe_log,
    hex_line,
)

HERE = app_dir()
LOG_ROOT = os.path.join(HERE, "日志")
KEEP = 8192
LOG_READ_MAX = 32
MODULES = (
    ("串口", MOD_UART, True),
    ("ADC", MOD_ADC, False),
    ("SPI", MOD_SPI, False),
    ("I2C", MOD_I2C, False),
    ("PWM", MOD_PWM, False),
)


PWM_TIM_HZ = 96000000


def pwm_regs(hz, duty):
    """把频率和占空比换成 TIM1 的 PSC、ARR、CCR。定时器时钟固定 96MHz。"""
    if hz < 1 or duty < 0 or duty > 100:
        return None
    total = PWM_TIM_HZ // hz
    if total < 2:
        return None
    psc_div = (total + 65535) // 65536
    if psc_div < 1 or psc_div > 65536:
        return None
    arr_div = total // psc_div
    if arr_div < 2:
        return None
    if arr_div > 65535:
        arr_div = 65535
    if duty == 0:
        ccr = 0
    elif duty == 100:
        ccr = arr_div
    else:
        ccr = (arr_div * duty) // 100
    return psc_div - 1, arr_div - 1, ccr


def pwm_text(body):
    """周期和高电平是微秒。只给界面一行字。"""
    if len(body) < 8:
        return None
    period, high = struct.unpack_from("<II", body, 0)
    if period == 0:
        return "输入：无信号"
    duty = (high * 100) // period
    if duty > 100:
        duty = 100
    return "输入：%u Hz，占空 %u%%" % (1000000 // period, duty)


def i2c_line(body):
    if not body:
        return "空帧"
    if body[0] == 1:
        if len(body) == 1:
            return "扫到 没有地址应答"
        return "扫到 " + " ".join("%02X" % b for b in body[1:])
    return "收 " + hex_line(body[1:])


def show_in_taskbar(win):
    """根窗口藏起来之后，每个新窗口仍要单独出现在任务栏。"""
    if sys.platform != "win32":
        return

    def apply():
        try:
            import ctypes
            hwnd = ctypes.windll.user32.GetParent(win.winfo_id())
            if not hwnd:
                hwnd = win.winfo_id()
            style = ctypes.windll.user32.GetWindowLongW(hwnd, -20)
            style = (style & ~0x00000080) | 0x00040000
            ctypes.windll.user32.SetWindowLongW(hwnd, -20, style)
            win.withdraw()
            win.deiconify()
        except Exception:
            pass

    win.after(50, apply)


class Session(object):
    """一条 USB 挂多个窗口。功能开或关记在这里，每个窗口都看得到收到的数据。"""

    def __init__(self, root):
        self.root = root
        self.windows = []
        self.next_index = 1
        self.busy = False
        self.closed = False
        self.port_name = ""
        self.want = {}
        for name, _mod, default in MODULES:
            self.want[name] = default
        self.pwm_text = ""
        self.logs = {}
        for name in ("串口", "ADC", "SPI", "I2C", "PWM", "升级", "日志"):
            self.logs[name] = ChannelLog(os.path.join(LOG_ROOT, name), name)
        self.link = Link(lambda text: self.broadcast("升级", text))
        self.root.after(80, self.pump)

    def broadcast(self, channel, text):
        line = self.logs[channel].write(text)
        for win in list(self.windows):
            win.ui.put((channel, line))

    def open_window(self):
        top = tk.Toplevel(self.root)
        Host(top, self)
        show_in_taskbar(top)

    def pump(self):
        if self.closed:
            return
        alive = []
        for win in self.windows:
            try:
                if win.root.winfo_exists():
                    alive.append(win)
            except tk.TclError:
                continue
        self.windows = alive
        for win in alive:
            win.flush()
        for frame in self.link.drain_side():
            self.on_frame(frame)
        for win in alive:
            win.draw_adc()
            win.sync_face()
        if self.link.ser is not None and not self.link.alive:
            for win in alive:
                win.status["text"] = "连接中断"
        self.root.after(80, self.pump)

    def on_frame(self, frame):
        kind = frame[2]
        body = frame[3:-2]
        if kind == CMD_UART_RX:
            self.broadcast("串口", "收 %s" % hex_line(body))
        elif kind == CMD_ADC:
            self.logs["ADC"].write("原始 " + hex_line(body))
            for win in list(self.windows):
                win.take_adc(body)
        elif kind == CMD_SPI:
            self.broadcast("SPI", "收 %s" % hex_line(body))
        elif kind == CMD_I2C:
            self.broadcast("I2C", i2c_line(body))
        elif kind == CMD_PWM_IN:
            text = pwm_text(body)
            if text is None or text == self.pwm_text:
                return
            self.pwm_text = text
            self.broadcast("PWM", text)
            for win in list(self.windows):
                win.show_pwm(text)
        else:
            self.broadcast("升级", "旁路帧 " + hex_line(frame))

    def shutdown(self):
        self.closed = True
        self.link.close()
        for item in self.logs.values():
            item.close()
        self.root.destroy()


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
    def __init__(self, root, session):
        self.root = root
        self.session = session
        self.link = session.link
        self.logs = session.logs
        index = session.next_index
        session.next_index += 1
        title = "协议分析仪" if index == 1 else "协议分析仪 - %d" % index
        self.root.title(title)
        self.root.geometry("980x680")
        self.root.minsize(860, 560)
        self.ui = queue.Queue()
        self.ports = []
        self.mod_lab = {}
        others = [win for win in session.windows if win is not self]
        if others:
            src = others[0]
            self.ema = list(src.ema)
            self.raw = [deque(ch, maxlen=KEEP) for ch in src.raw]
            self.smooth = [deque(ch, maxlen=KEEP) for ch in src.smooth]
            self.adc_dt_us = src.adc_dt_us
        else:
            self.ema = [None, None, None]
            self.raw = [deque(maxlen=KEEP) for _ in range(3)]
            self.smooth = [deque(maxlen=KEEP) for _ in range(3)]
            self.adc_dt_us = sample_us(3)
        self.show_volt = tk.BooleanVar(value=True)
        self.alpha = tk.DoubleVar(value=0.25)
        self.hex_mode = tk.BooleanVar(value=True)
        session.windows.append(self)

        top = ttk.Frame(root, padding=6)
        top.pack(fill=tk.X)
        ttk.Label(top, text="USB").pack(side=tk.LEFT)
        self.port_box = ttk.Combobox(top, width=42, state="readonly")
        self.port_box.pack(side=tk.LEFT, padx=4)
        ttk.Button(top, text="刷新", command=self.refresh).pack(side=tk.LEFT)
        self.conn_btn = ttk.Button(top, text="连接", command=self.toggle)
        self.conn_btn.pack(side=tk.LEFT, padx=4)
        ttk.Button(top, text="新窗口", command=session.open_window).pack(side=tk.LEFT)
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
        self._build_devlog()
        self.note_here("串口", "日志目录：%s" % LOG_ROOT)
        self.note_here("串口", "请连接 1A86:FE0C。串口默认打开，其它功能点「打开」才发给板子。切页不会关掉已经打开的功能。")
        self.refresh()
        self.sync_face()
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def note(self, channel, text):
        self.session.broadcast(channel, text)

    def note_here(self, channel, text):
        line = self.logs[channel].write(text)
        self.ui.put((channel, line))

    def _switch(self, bar, name, mod):
        ttk.Button(bar, text="打开", command=lambda: self.module_set(mod, OP_ON, name)).pack(side=tk.LEFT)
        ttk.Button(bar, text="关闭", command=lambda: self.module_set(mod, OP_OFF, name)).pack(side=tk.LEFT, padx=4)
        lab = ttk.Label(bar, width=4)
        lab.pack(side=tk.LEFT, padx=(0, 8))
        self.mod_lab[name] = lab

    def _view(self, parent, height=8, expand=True):
        box = scrolledtext.ScrolledText(parent, height=height, font=("Consolas", 9))
        box.pack(fill=tk.BOTH, expand=expand, padx=4, pady=4)
        return box

    def _build_uart(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="串口")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        self._switch(bar, "串口", MOD_UART)
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
        self._switch(bar, "ADC", MOD_ADC)
        ttk.Checkbutton(bar, text="显示伏特（×11）", variable=self.show_volt).pack(side=tk.LEFT, padx=8)
        ttk.Label(bar, text="EMA").pack(side=tk.LEFT)
        ttk.Scale(bar, from_=0.05, to=1.0, variable=self.alpha, length=120).pack(side=tk.LEFT, padx=4)
        self.scope = AdcScope(page)
        self.scope.pack(fill=tk.BOTH, expand=True)
        self.view_adc = self._view(page, 4, expand=False)

    def _build_spi(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="SPI")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        self._switch(bar, "SPI", MOD_SPI)
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
        self._switch(bar, "I2C", MOD_I2C)
        self.i2c_role = ttk.Combobox(bar, width=6, state="readonly", values=["主机", "从机"])
        self.i2c_role.set("主机")
        self.i2c_role.pack(side=tk.LEFT)
        self.i2c_spd = ttk.Combobox(bar, width=6, state="readonly", values=["100k", "400k"])
        self.i2c_spd.set("100k")
        self.i2c_spd.pack(side=tk.LEFT, padx=4)
        ttk.Button(bar, text="速率", command=self.i2c_speed).pack(side=tk.LEFT)
        ttk.Label(bar, text="地址").pack(side=tk.LEFT, padx=4)
        self.i2c_addr = ttk.Entry(bar, width=8)
        self.i2c_addr.insert(0, "50")
        self.i2c_addr.pack(side=tk.LEFT)
        ttk.Button(bar, text="切换", command=self.i2c_cfg).pack(side=tk.LEFT, padx=4)
        ttk.Button(bar, text="扫描地址", command=self.i2c_scan).pack(side=tk.LEFT)
        ttk.Label(bar, text="读长").pack(side=tk.LEFT, padx=4)
        self.i2c_len = ttk.Entry(bar, width=4)
        self.i2c_len.insert(0, "10")
        self.i2c_len.pack(side=tk.LEFT)
        ttk.Button(bar, text="读取", command=self.i2c_read).pack(side=tk.LEFT, padx=4)
        self.i2c_send = ttk.Entry(bar)
        self.i2c_send.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=4)
        ttk.Button(bar, text="发送", command=self.i2c_tx).pack(side=tk.LEFT)
        self.view_i2c = self._view(page, 16)

    def _build_pwm(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="PWM")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        self._switch(bar, "PWM", MOD_PWM)
        ttk.Label(bar, text="频率Hz").pack(side=tk.LEFT)
        self.pwm_hz = ttk.Entry(bar, width=8)
        self.pwm_hz.insert(0, "1000")
        self.pwm_hz.pack(side=tk.LEFT, padx=4)
        ttk.Label(bar, text="占空%").pack(side=tk.LEFT)
        self.pwm_duty = ttk.Entry(bar, width=6)
        self.pwm_duty.insert(0, "50")
        self.pwm_duty.pack(side=tk.LEFT, padx=4)
        ttk.Button(bar, text="输出", command=self.pwm_out).pack(side=tk.LEFT)
        self.pwm_info = ttk.Label(page, text="输入：还没有")
        self.pwm_info.pack(anchor="w", padx=4)
        if self.session.pwm_text:
            self.pwm_info["text"] = self.session.pwm_text
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
        self.note_here("升级", "选择加密.hex。不要点确认，新面自己留下。密钥：%s" % key_path())

    def _build_devlog(self):
        page = ttk.Frame(self.book)
        self.book.add(page, text="日志")
        bar = ttk.Frame(page)
        bar.pack(fill=tk.X, padx=4, pady=4)
        ttk.Button(bar, text="读最新一条", command=lambda: self.run_log(1)).pack(side=tk.LEFT)
        ttk.Label(bar, text="最近").pack(side=tk.LEFT, padx=(8, 0))
        self.log_count = ttk.Entry(bar, width=6)
        self.log_count.insert(0, "8")
        self.log_count.pack(side=tk.LEFT, padx=4)
        ttk.Label(bar, text="条，最多 %u" % LOG_READ_MAX).pack(side=tk.LEFT)
        ttk.Button(bar, text="读取", command=lambda: self.run_log(self._log_count())).pack(side=tk.LEFT, padx=4)
        self.view_log = self._view(page, 16)
        self.note_here("日志", "发 0xE6 读设备日志，序号 0 是最新一条。设备回 0x62 的 32 字节，这里按启动、紧急、日常解释。一次最多 %u 条。读日志不关其它功能。" % LOG_READ_MAX)

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
            self.session.port_name = ""
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
        self.session.port_name = item.device
        self.note("串口", "已连接 %s" % item.device)
        self.push_wanted()

    def push_wanted(self):
        """连接之后，只把上位机记成「开」的功能再发一次。关着的不发。"""

        def work():
            for name, mod, _default in MODULES:
                if self.session.want[name]:
                    self.ctrl(mod, OP_ON, name)

        threading.Thread(target=work, daemon=True).start()

    def ctrl(self, mod, op, name):
        word = "打开" if op == OP_ON else "关闭"
        kind, body = self.link.transact(CMD_CTRL, bytes([mod, op]), 0.4)
        if kind is None:
            self.note(name, "%s %s没有应答" % (name, word))
            return
        if kind == CMD_NACK:
            code = body[1] if len(body) > 1 else -1
            self.note(name, "%s %s被拒绝：%s（板端还没接这个开关时会这样）" % (
                name, word, RES_NAME.get(code, str(code))))
            return
        self.note(name, "%s已%s" % (name, word))

    def module_set(self, mod, op, name):
        self.session.want[name] = (op == OP_ON)
        if self.link.ser is None:
            self.note(name, "%s已记为%s，连接后发出" % (name, "打开" if op == OP_ON else "关闭"))
            return
        self.module_cmd(mod, op, name)

    def module_cmd(self, mod, op, name):
        if self.link.ser is None:
            self.note(name, "还没有连接")
            return
        threading.Thread(target=lambda: self.ctrl(mod, op, name), daemon=True).start()

    def need_open(self, name):
        if self.session.want.get(name):
            return True
        self.note(name, "这个功能还关着，先点打开")
        return False

    def sync_face(self):
        linked = self.link.ser is not None and self.link.alive
        self.conn_btn["text"] = "断开" if self.link.ser is not None else "连接"
        if linked:
            self.status["text"] = self.session.port_name or "已连接"
        elif self.link.ser is None:
            self.status["text"] = "未连接"
        for name, lab in self.mod_lab.items():
            lab["text"] = "开" if self.session.want.get(name) else "关"

    def flush(self):
        views = {"串口": self.view_uart, "ADC": self.view_adc, "SPI": self.view_spi,
                 "I2C": self.view_i2c, "PWM": self.view_pwm, "升级": self.view_ota,
                 "日志": self.view_log}
        while True:
            try:
                channel, line = self.ui.get_nowait()
            except queue.Empty:
                break
            box = views.get(channel)
            if box is not None:
                box.insert(tk.END, line + "\n")
                box.see(tk.END)

    def run_bg(self, func):
        if self.session.busy:
            self.note("升级", "上一步还没结束")
            return
        if self.link.ser is None:
            self.note("升级", "还没有连接")
            return
        self.session.busy = True

        def wrap():
            try:
                func()
            except Exception as exc:
                self.note("升级", "中断：%s" % exc)
            self.session.busy = False

        threading.Thread(target=wrap, daemon=True).start()

    def run_log(self, count):
        if self.session.busy:
            self.note("日志", "上一步还没结束")
            return
        if self.link.ser is None:
            self.note("日志", "还没有连接")
            return
        self.session.busy = True

        def wrap():
            try:
                self.do_read_logs(count)
            except Exception as exc:
                self.note("日志", "中断：%s" % exc)
            self.session.busy = False

        threading.Thread(target=wrap, daemon=True).start()

    def _log_count(self):
        try:
            count = int(self.log_count.get().strip())
        except ValueError:
            count = 8
        count = self._cap_log_count(count)
        self.log_count.delete(0, tk.END)
        self.log_count.insert(0, str(count))
        return count

    def _cap_log_count(self, count):
        """设备里能有两千多条。一次读太多会把这条 USB 占住，所以这里卡住。"""
        if count < 1:
            return 1
        if count > LOG_READ_MAX:
            self.note("日志", "一次最多读 %u 条，多出来的不读" % LOG_READ_MAX)
            return LOG_READ_MAX
        return count

    def do_read_logs(self, count):
        count = self._cap_log_count(count)
        self.note("日志", "—— 读取 %u 条 ——" % count)
        kind, body = self.link.transact(CMD_LOG, struct.pack("<I", 0), 1.0)
        if not self._take_log(kind, body, 0):
            return
        seq = struct.unpack_from("<I", body)[0]
        left = count - 1
        while left > 0 and seq > 1:
            seq -= 1
            left -= 1
            kind, body = self.link.transact(CMD_LOG, struct.pack("<I", seq), 1.0)
            if not self._take_log(kind, body, seq):
                return

    def _take_log(self, kind, body, seq):
        if kind is None:
            self.note("日志", "序号 %s 没有回复" % ("最新" if seq == 0 else str(seq)))
            return False
        if kind == CMD_NACK:
            code = body[1] if len(body) > 1 else -1
            if code == 4:
                if seq == 0:
                    self.note("日志", "设备里还没有日志")
                else:
                    self.note("日志", "序号 %u 没有记录，再早的也停在这里" % seq)
            else:
                self.note("日志", "被拒绝：%s" % RES_NAME.get(code, str(code)))
            return False
        if kind != CMD_LOG_RSP or len(body) != 32:
            self.note("日志", "回复不是一条 32 字节日志")
            return False
        self.note("日志", describe_log(body))
        self.note("日志", "原始 " + hex_line(body))
        return True

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
        if not self.need_open("串口"):
            return
        code = BAUD_CODE.get(self.baud.get())
        if code is None or self.link.ser is None:
            return
        threading.Thread(target=lambda: self.link.transact(CMD_UART_LINE, bytes([code]), 0.6), daemon=True).start()
        self.note("串口", "请求波特率 %s" % self.baud.get())

    def send_uart(self):
        if not self.need_open("串口"):
            return
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

    def i2c_speed(self):
        fast = 0 if self.i2c_spd.get() == "100k" else 1
        self.note("I2C", "请求速率 %s" % self.i2c_spd.get())
        self.module_send(CMD_I2C_CFG, bytes([0, 0, 4, fast]), "I2C")

    def i2c_cfg(self):
        role = 0 if self.i2c_role.get() == "主机" else 1
        addr = self._addr(self.i2c_addr.get())
        self.note("I2C", "请求%s，地址 0x%02X" % (self.i2c_role.get(), addr))
        self.module_send(CMD_I2C_CFG, bytes([role, addr, 0]), "I2C")

    def i2c_scan(self):
        if self.i2c_role.get() != "主机":
            self.note("I2C", "扫描要先切到主机")
            return
        self.note("I2C", "请求扫描 7 位地址")
        self.module_send(CMD_I2C_CFG, bytes([0, 0, 1]), "I2C")

    def i2c_read(self):
        if self.i2c_role.get() != "主机":
            self.note("I2C", "读取长度只在主机用。从机有数据时自己拉低通知脚")
            return
        try:
            n = int(self.i2c_len.get())
        except ValueError:
            self.note("I2C", "读长要是 1 到 40 的整数")
            return
        if n < 1 or n > 40:
            self.note("I2C", "读长要是 1 到 40 的整数")
            return
        addr = self._addr(self.i2c_addr.get())
        self.note("I2C", "请求读取 %u 字节" % n)
        self.module_send(CMD_I2C_CFG, bytes([0, addr, 3, n]), "I2C")

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
        if hz < 1 or duty < 0 or duty > 100:
            self.note("PWM", "频率至少 1Hz，占空比是 0 到 100")
            return
        regs = pwm_regs(hz, duty)
        if regs is None:
            self.note("PWM", "这个频率配不出来，最高大约 48MHz")
            return
        psc, arr, ccr = regs
        actual = PWM_TIM_HZ // ((psc + 1) * (arr + 1))
        self.note("PWM", "请求 %u Hz，占空 %u%%，实际约 %u Hz" % (hz, duty, actual))
        self.module_send(CMD_PWM_OUT, struct.pack("<HHH", psc, arr, ccr), "PWM")

    def module_send(self, cmd, data, name):
        if not self.need_open(name):
            return
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

    def take_adc(self, body):
        if len(body) < 4 or ((len(body) - 2) % 2) != 0:
            self.note("ADC", "样本长度不对：" + hex_line(body))
            return
        nch = body[1]
        if nch < 1 or nch > 3:
            self.note("ADC", "通道数 %u 不在 1 到 3" % nch)
            return
        self.adc_dt_us = sample_us(nch)
        words = struct.unpack_from("<%uH" % ((len(body) - 2) // 2), body, 2)
        alpha = float(self.alpha.get())
        for index, sample in enumerate(words):
            ch = index % nch
            self.raw[ch].append(sample)
            self.smooth[ch].append(ema_step(self.ema[ch], sample, alpha))
            self.ema[ch] = self.smooth[ch][-1]

    def draw_adc(self):
        self.scope.draw(self.smooth, self.show_volt.get(), self.adc_dt_us)

    def show_pwm(self, text):
        self.pwm_info["text"] = text

    def on_close(self):
        if self in self.session.windows:
            self.session.windows.remove(self)
        self.root.destroy()
        if not self.session.windows:
            self.session.shutdown()


def main():
    if sys.platform == "win32":
        try:
            import ctypes
            ctypes.windll.shcore.SetProcessDpiAwareness(1)
        except Exception:
            pass
    root = tk.Tk()
    root.withdraw()
    session = Session(root)
    session.open_window()
    root.mainloop()


if __name__ == "__main__":
    main()
