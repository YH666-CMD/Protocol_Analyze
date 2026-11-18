# -*- coding: utf-8 -*-
"""临时明文升级发送。走芯片的 USB 虚拟串口，帧和固件 frame.h 一致。"""

import datetime
import os
import queue
import struct
import threading
import tkinter as tk
from tkinter import filedialog, scrolledtext, ttk
import zlib

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    serial = None

SOF = 0xA5
FRAME_MAX = 64
CMD_ACK = 0x01
CMD_NACK = 0x02
CMD_INFO_RSP = 0x61
CMD_INFO = 0xE0
CMD_START = 0xE1
CMD_DATA = 0xE2
CMD_END = 0xE3
CMD_CONFIRM = 0xE4
CMD_ABORT = 0xE5
DATA_MAX = 48
SLOT_SIZE = 30 * 1024
HDR_OFS = 0x100
IMG_MAGIC = 0x31474D49
LINK_OFS = (0x800, 0x8000)
VID_CDC = (0x1A86, 0xFE0C)
VID_CH340 = (0x1A86, 0x7523)

RES_NAME = {
    0: "成功",
    1: "参数不对",
    2: "正忙",
    3: "空间不够",
    4: "状态不对",
    5: "这次调用超时",
}
STATE_NAME = {0: "空", 1: "正在写", 2: "待试运行", 3: "良好", 4: "坏"}
HERE = os.path.dirname(os.path.abspath(__file__))
LOG_DIR = os.path.join(HERE, "日志")


def crc16(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def pack(cmd, data=b""):
    body = bytes([SOF, 5 + len(data), cmd]) + bytes(data)
    c = crc16(body)
    return body + bytes((c & 0xFF, c >> 8))


def hex_line(data):
    return data.hex(" ").upper()


def slot_name(value):
    if value == 0:
        return "A"
    if value == 1:
        return "B"
    if value == 0xFF:
        return "无"
    return "0x%02X" % value


class Port(object):
    def __init__(self, log):
        self.log = log
        self.ser = None
        self.alive = False
        self.rx = queue.Queue()
        self.thread = None

    def open(self, name):
        self.close()
        self.ser = serial.Serial(name, 115200, timeout=0.05)
        self.alive = True
        self.thread = threading.Thread(target=self._read, daemon=True)
        self.thread.start()

    def close(self):
        self.alive = False
        ser = self.ser
        self.ser = None
        if ser is not None:
            try:
                ser.close()
            except Exception:
                pass

    def _read(self):
        buf = bytearray()
        while self.alive and self.ser is not None:
            try:
                chunk = self.ser.read(64)
            except Exception as exc:
                self.log("串口读失败：%s" % exc)
                self.alive = False
                return
            if not chunk:
                continue
            buf.extend(chunk)
            while True:
                frame = self._take(buf)
                if frame is None:
                    break
                self.rx.put(frame)

    def _take(self, buf):
        while buf and buf[0] != SOF:
            self.log("丢掉非帧头字节 %02X" % buf[0])
            del buf[0]
        if len(buf) < 2:
            return None
        length = buf[1]
        if length < 5 or length > FRAME_MAX:
            self.log("长度字节 %02X 非法，丢掉帧头" % length)
            del buf[0]
            return None
        if len(buf) < length:
            return None
        raw = bytes(buf[:length])
        del buf[:length]
        if crc16(raw[:-2]) != (raw[-2] | (raw[-1] << 8)):
            self.log("CRC 不对，整帧丢掉：%s" % hex_line(raw))
            return None
        return raw

    def transact(self, cmd, data=b"", timeout=1.0):
        raw = pack(cmd, data)
        self.log("发 %s" % hex_line(raw))
        self.ser.write(raw)
        end = datetime.datetime.now().timestamp() + timeout
        while datetime.datetime.now().timestamp() < end:
            remain = end - datetime.datetime.now().timestamp()
            try:
                frame = self.rx.get(timeout=max(0.05, remain))
            except queue.Empty:
                continue
            self.log("收 %s" % hex_line(frame))
            kind = frame[2]
            body = frame[3:-2]
            if kind in (CMD_ACK, CMD_NACK) and body and body[0] == cmd:
                return kind, body
            if cmd == CMD_INFO and kind == CMD_INFO_RSP:
                return kind, body
            self.log("这帧不是本次命令的回复，已记下并继续等")
        self.log("超时：%s 秒内没有等到回复" % timeout)
        return None, b""


class App(object):
    def __init__(self, root):
        self.root = root
        self.root.title("升级功能测试")
        self.lines = queue.Queue()
        self.busy = False
        self.port = Port(self.log)
        self.ports = []
        os.makedirs(LOG_DIR, exist_ok=True)
        stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        self.log_path = os.path.join(LOG_DIR, "升级_%s.txt" % stamp)
        self.log_file = open(self.log_path, "a", encoding="utf-8")

        top = ttk.Frame(root, padding=8)
        top.pack(fill=tk.X)
        ttk.Label(top, text="串口").pack(side=tk.LEFT)
        self.port_box = ttk.Combobox(top, width=42, state="readonly")
        self.port_box.pack(side=tk.LEFT, padx=4)
        ttk.Button(top, text="刷新", command=self.refresh).pack(side=tk.LEFT)
        self.conn_btn = ttk.Button(top, text="连接", command=self.toggle)
        self.conn_btn.pack(side=tk.LEFT, padx=4)

        mid = ttk.Frame(root, padding=(8, 0, 8, 8))
        mid.pack(fill=tk.X)
        ttk.Label(mid, text="明文 bin").pack(side=tk.LEFT)
        self.path = tk.StringVar()
        ttk.Entry(mid, textvariable=self.path, width=46).pack(side=tk.LEFT, padx=4)
        ttk.Button(mid, text="选择", command=self.browse).pack(side=tk.LEFT)
        self.target = tk.StringVar(value="自动")
        ttk.Combobox(mid, textvariable=self.target, width=8, state="readonly",
                     values=("自动", "A", "B")).pack(side=tk.LEFT, padx=6)

        buttons = ttk.Frame(root, padding=(8, 0, 8, 8))
        buttons.pack(fill=tk.X)
        ttk.Button(buttons, text="查询", command=lambda: self.run(self.do_info)).pack(side=tk.LEFT)
        ttk.Button(buttons, text="升级", command=lambda: self.run(self.do_upgrade)).pack(side=tk.LEFT, padx=4)
        ttk.Button(buttons, text="确认", command=lambda: self.run(self.do_confirm)).pack(side=tk.LEFT)
        ttk.Button(buttons, text="放弃", command=lambda: self.run(self.do_abort)).pack(side=tk.LEFT, padx=4)

        self.text = scrolledtext.ScrolledText(root, height=24, font=("Consolas", 10))
        self.text.pack(fill=tk.BOTH, expand=True, padx=8, pady=(0, 8))
        self.log("日志文件：%s" % self.log_path)
        self.log("请选芯片的 USB 串口（1A86:FE0C）。CH340 连的是普通串口，当前固件不在那边解析升级帧。")
        self.refresh()
        self.root.after(100, self._drain)
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def log(self, message):
        line = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3] + "  " + message
        self.lines.put(line)

    def _drain(self):
        while True:
            try:
                line = self.lines.get_nowait()
            except queue.Empty:
                break
            self.text.insert(tk.END, line + "\n")
            self.text.see(tk.END)
            self.log_file.write(line + "\n")
            self.log_file.flush()
        self.root.after(100, self._drain)

    def refresh(self):
        self.ports = []
        labels = []
        if serial is None:
            self.log("没有 pyserial。先执行：py -3 -m pip install pyserial")
            return
        for item in serial.tools.list_ports.comports():
            mark = ""
            ident = (item.vid, item.pid)
            if ident == VID_CDC:
                mark = "  [设备USB]"
            elif ident == VID_CH340 or "CH340" in (item.description or "").upper():
                mark = "  [CH340，不要选这个做升级]"
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
        if self.port.ser is not None:
            self.port.close()
            self.conn_btn["text"] = "连接"
            self.log("已断开")
            return
        item = self.selected()
        if item is None:
            self.log("没有选中串口")
            return
        if (item.vid, item.pid) == VID_CH340 or "CH340" in (item.description or "").upper():
            self.log("这个口是 CH340。升级帧发到这里，芯片的 USB 协议收不到。请改选带 [设备USB] 的口。")
            return
        try:
            self.port.open(item.device)
        except Exception as exc:
            self.log("打开失败：%s" % exc)
            return
        self.conn_btn["text"] = "断开"
        self.log("已连接 %s" % item.device)

    def browse(self):
        path = filedialog.askopenfilename(filetypes=[("bin", "*.bin"), ("全部", "*.*")])
        if path:
            self.path.set(path)

    def run(self, func):
        if self.busy:
            self.log("上一步还没结束")
            return
        if self.port.ser is None:
            self.log("还没有连接")
            return
        self.busy = True
        threading.Thread(target=self._work, args=(func,), daemon=True).start()

    def _work(self, func):
        try:
            func()
        except Exception as exc:
            self.log("中断：%s" % exc)
        self.busy = False

    def need_ack(self, cmd, data=b"", timeout=1.0, step=""):
        kind, body = self.port.transact(cmd, data, timeout)
        if kind is None:
            self.log("%s失败：没有回复" % step)
            return None
        if len(body) < 2:
            self.log("%s失败：回复太短 %s" % (step, hex_line(body)))
            return None
        code = body[1]
        if kind == CMD_NACK:
            extra = ""
            if len(body) >= 6:
                extra = "，期望偏移 %u" % struct.unpack_from("<I", body, 2)[0]
            self.log("%s被拒绝：%s%s" % (step, RES_NAME.get(code, str(code)), extra))
            return None
        if code != 0:
            self.log("%s回了 ACK，但结果是 %s" % (step, RES_NAME.get(code, str(code))))
            return None
        self.log("%s成功" % step)
        return body

    def do_info(self):
        self.log("—— 查询 ——")
        kind, body = self.port.transact(CMD_INFO, b"", 1.0)
        if kind is None:
            self.log("查询失败。设备若回了 NACK 结果 4，表示升级命令还没接上。")
            return
        if kind == CMD_NACK:
            code = body[1] if len(body) > 1 else -1
            self.log("查询被拒绝：%s" % RES_NAME.get(code, str(code)))
            return
        if kind != CMD_INFO_RSP or len(body) != 13:
            self.log("查询回复长度不是 13 字节，实际 %u" % len(body))
            return
        run, version, st_a, st_b, trial, count, ofs = struct.unpack("<BIBBBBI", body)
        self.log("当前面 %s，版本 %u，A=%s，B=%s，试运行 %s，次数 %u，已写偏移 %u" % (
            slot_name(run), version, STATE_NAME.get(st_a, st_a), STATE_NAME.get(st_b, st_b),
            slot_name(trial), count, ofs))

    def load_bin(self):
        path = self.path.get().strip()
        if not path:
            self.log("还没有选择 bin")
            return None
        data = open(path, "rb").read()
        self.log("文件 %s，%u 字节" % (path, len(data)))
        if len(data) == 0 or len(data) > SLOT_SIZE:
            self.log("长度必须是 1 到 %u" % SLOT_SIZE)
            return None
        if len(data) < HDR_OFS + 16:
            self.log("文件比镜像头还短，偏移 0x100 没有头")
            return None
        magic, link, version, img_end = struct.unpack_from("<IIII", data, HDR_OFS)
        self.log("镜像头 magic=%08X link=%X version=%u img_end=%X" % (magic, link, version, img_end))
        if magic != IMG_MAGIC:
            self.log("魔术字不是 IMG1，不发送")
            return None
        crc = zlib.crc32(data) & 0xFFFFFFFF
        self.log("整文件 CRC32 %08X" % crc)
        return data, version, link, crc

    def choose_target(self, link):
        pick = self.target.get()
        if pick == "A":
            target = 0
        elif pick == "B":
            target = 1
        else:
            self.log("自动选择：先查询当前面")
            kind, body = self.port.transact(CMD_INFO, b"", 1.0)
            if kind != CMD_INFO_RSP or len(body) != 13:
                self.log("查不到当前面，自动选择失败。可以改成手动指定 A 或 B。")
                return None
            run = body[0]
            if run not in (0, 1):
                self.log("当前面是 %s，不能决定升级目标" % slot_name(run))
                return None
            target = run ^ 1
            self.log("当前在 %s，升级目标是 %s" % (slot_name(run), slot_name(target)))
        if link != LINK_OFS[target]:
            self.log("这份 bin 的 link_ofs 是 %X，目标面 %s 应该是 %X，不发送" % (
                link, slot_name(target), LINK_OFS[target]))
            return None
        return target

    def do_upgrade(self):
        self.log("—— 升级 ——")
        loaded = self.load_bin()
        if loaded is None:
            return
        data, version, link, crc = loaded
        target = self.choose_target(link)
        if target is None:
            return
        start = struct.pack("<BIII", target, len(data), crc, version)
        if self.need_ack(CMD_START, start, 3.0, "开始") is None:
            return
        offset = 0
        retries = 0
        while offset < len(data):
            chunk = data[offset:offset + DATA_MAX]
            payload = struct.pack("<I", offset) + chunk
            kind, body = self.port.transact(CMD_DATA, payload, 3.0)
            if kind == CMD_ACK and len(body) >= 2 and body[1] == 0:
                offset += len(chunk)
                retries = 0
                if offset == len(chunk) or offset == len(data) or (offset % (DATA_MAX * 20)) == 0:
                    self.log("已写入 %u / %u" % (offset, len(data)))
                continue
            expect = None
            if kind == CMD_NACK and len(body) >= 6:
                expect = struct.unpack_from("<I", body, 2)[0]
                self.log("数据被拒绝，设备期望偏移 %u，当前发送偏移 %u" % (expect, offset))
            else:
                self.log("偏移 %u 没有成功" % offset)
            retries += 1
            if retries > 5:
                self.log("同一位置连续失败，升级停止。已确认写入到偏移 %u" % offset)
                return
            if expect is not None and expect <= len(data):
                offset = expect
            self.log("准备从偏移 %u 再发，第 %u 次" % (offset, retries))
        if self.need_ack(CMD_END, b"", 8.0, "结束") is None:
            return
        self.log("结束已确认。设备这时应该重启。现在没有看门狗，USB 若还在，请手动按复位。")
        self.log("复位后重新连接，再点确认。确认会把试运行面写成正式运行面。")

    def do_confirm(self):
        self.log("—— 确认 ——")
        self.need_ack(CMD_CONFIRM, b"", 2.0, "确认")

    def do_abort(self):
        self.log("—— 放弃 ——")
        self.need_ack(CMD_ABORT, b"", 2.0, "放弃")

    def on_close(self):
        self.port.close()
        self.log_file.close()
        self.root.destroy()


def main():
    root = tk.Tk()
    App(root)
    root.mainloop()


if __name__ == "__main__":
    main()
