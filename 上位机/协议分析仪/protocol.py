# -*- coding: utf-8 -*-
"""USB 帧。和固件 frame.h 同一套：A5、长度、指令、CRC-16/CCITT-FALSE。"""

import queue
import threading
import time

try:
    import serial
except ImportError:
    serial = None

SOF = 0xA5
FRAME_MAX = 64
DATA_MAX = 59

CMD_ACK = 0x01
CMD_NACK = 0x02
CMD_UART_RX = 0x10
CMD_PWM_IN = 0x20
CMD_ADC = 0x30
CMD_SPI = 0x40
CMD_I2C = 0x50
CMD_INFO_RSP = 0x61
CMD_PING = 0x80
CMD_CTRL = 0x81
CMD_UART_TX = 0x90
CMD_UART_LINE = 0x91
CMD_PWM_OUT = 0xA0
CMD_ADC_CFG = 0xB0
CMD_SPI_CFG = 0xC0
CMD_I2C_CFG = 0xD0
CMD_INFO = 0xE0
CMD_START = 0xE1
CMD_DATA = 0xE2
CMD_END = 0xE3

MOD_UART = 0x01
MOD_PWM = 0x02
MOD_ADC = 0x03
MOD_SPI = 0x04
MOD_I2C = 0x05
OP_OFF = 0x00
OP_ON = 0x01

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
BAUD_CODE = {
    "115200": 1,
    "9600": 2,
    "19200": 3,
    "38400": 4,
    "57600": 5,
    "230400": 6,
    "460800": 7,
    "921600": 8,
}


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


def explain(frame):
    kind = frame[2]
    body = frame[3:-2]
    if kind == CMD_INFO_RSP and len(body) == 13:
        import struct
        run, version, st_a, st_b, trial, count, addr = struct.unpack("<BIBBBBI", body)
        return ("正在跑 %s 面，版本 %u。A %s，B %s。试运行 %s，次数 %u。下一地址 %08X。"
                % (slot_name(run), version,
                   STATE_NAME.get(st_a, str(st_a)), STATE_NAME.get(st_b, str(st_b)),
                   slot_name(trial), count, addr))
    if kind in (CMD_ACK, CMD_NACK) and len(body) >= 2:
        word = "同意" if kind == CMD_ACK else "拒绝"
        result = RES_NAME.get(body[1], "结果 %u" % body[1])
        extra = ""
        if kind == CMD_NACK and len(body) >= 6:
            import struct
            extra = "，设备要的地址 %08X" % struct.unpack_from("<I", body, 2)[0]
        return "%s 指令 %02X：%s%s" % (word, body[0], result, extra)
    if kind == CMD_UART_RX:
        return "串口收到 %u 字节" % len(body)
    return "指令 %02X，数据 %u 字节" % (kind, len(body))


class Link(object):
    """后台拆帧。应答交给正在等的命令，其余帧放进 side，给界面取。"""

    def __init__(self, log):
        self.log = log
        self.ser = None
        self.alive = False
        self.inbox = queue.Queue()
        self.side = queue.Queue()
        self.reply_q = None
        self.wait_cmd = None
        self._io = threading.Lock()

    def open(self, name):
        self.close()
        self.ser = serial.Serial(name, 115200, timeout=0.05)
        self.alive = True
        threading.Thread(target=self._read, daemon=True).start()
        threading.Thread(target=self._dispatch, daemon=True).start()

    def close(self):
        self.alive = False
        self.reply_q = None
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
                self.log("USB 读失败：%s" % exc)
                self.alive = False
                return
            if not chunk:
                continue
            buf.extend(chunk)
            while True:
                frame = self._take(buf)
                if frame is None:
                    break
                self.inbox.put(frame)

    def _take(self, buf):
        while buf and buf[0] != SOF:
            del buf[0]
        if len(buf) < 2:
            return None
        length = buf[1]
        if length < 5 or length > FRAME_MAX:
            del buf[0]
            return None
        if len(buf) < length:
            return None
        raw = bytes(buf[:length])
        del buf[:length]
        if crc16(raw[:-2]) != (raw[-2] | (raw[-1] << 8)):
            self.log("CRC 不对：%s" % hex_line(raw))
            return None
        return raw

    def _match(self, frame):
        cmd = self.wait_cmd
        if cmd is None:
            return False
        kind = frame[2]
        body = frame[3:-2]
        if kind in (CMD_ACK, CMD_NACK) and body and body[0] == cmd:
            return True
        if cmd == CMD_INFO and kind == CMD_INFO_RSP:
            return True
        return False

    def _dispatch(self):
        while self.alive:
            try:
                frame = self.inbox.get(timeout=0.2)
            except queue.Empty:
                continue
            q = self.reply_q
            if q is not None and self._match(frame):
                q.put(frame)
            else:
                self.side.put(frame)

    def write(self, raw):
        with self._io:
            self.ser.write(raw)

    def transact(self, cmd, data=b"", timeout=1.0, want_reply=True):
        raw = pack(cmd, data)
        self.log("发 %s" % hex_line(raw))
        box = queue.Queue()
        self.wait_cmd = cmd
        self.reply_q = box
        try:
            self.write(raw)
            end = time.time() + timeout
            while time.time() < end:
                try:
                    frame = box.get(timeout=0.05)
                except queue.Empty:
                    if not want_reply:
                        return "silent", b""
                    continue
                self.log("收 %s" % hex_line(frame))
                self.log(explain(frame))
                return frame[2], frame[3:-2]
            if not want_reply:
                return "silent", b""
            self.log("超时：%s 秒内没有应答" % timeout)
            return None, b""
        finally:
            self.reply_q = None
            self.wait_cmd = None

    def drain_side(self):
        frames = []
        while True:
            try:
                frames.append(self.side.get_nowait())
            except queue.Empty:
                return frames
