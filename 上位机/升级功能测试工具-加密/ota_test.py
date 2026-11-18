# -*- coding: utf-8 -*-
"""加密升级发送。HEX 里是密文，按地址原样发出去。开始命令里的 CRC 是解密后的明文。"""

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
SLOT_LEN = 30 * 1024
SLOT_A = 0x08000800
SLOT_B = 0x08008000
HDR_OFS = 0x100
IMG_MAGIC = 0x31474D49
LINK_OFS = {SLOT_A: 0x800, SLOT_B: 0x8000}
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
KEY_FILE = os.path.normpath(os.path.join(HERE, "..", "..", "工具", "加密工具", "密钥.txt"))


def _xtime(value):
    return ((value << 1) ^ 0x1B) & 0xFF if value & 0x80 else (value << 1) & 0xFF


def _mul(left, right):
    product = 0
    for _ in range(8):
        if right & 1:
            product ^= left
        left = _xtime(left)
        right >>= 1
    return product


def _gf_inv(value):
    if value == 0:
        return 0
    result = 1
    base = value
    exp = 254
    while exp:
        if exp & 1:
            result = _mul(result, base)
        base = _mul(base, base)
        exp >>= 1
    return result


def _sbox_of(value):
    value = _gf_inv(value)
    mixed = value
    for _ in range(4):
        value = ((value << 1) | (value >> 7)) & 0xFF
        mixed ^= value
    return mixed ^ 0x63


_SBOX = tuple(_sbox_of(i) for i in range(256))
_INV_SBOX = [0] * 256
for _index, _value in enumerate(_SBOX):
    _INV_SBOX[_value] = _index
_INV_SBOX = tuple(_INV_SBOX)


def _expand_key(key):
    words = [list(key[i:i + 4]) for i in range(0, 16, 4)]
    rcon = 1
    for index in range(4, 44):
        temp = words[index - 1][:]
        if index % 4 == 0:
            temp = temp[1:] + temp[:1]
            temp = [_SBOX[byte] for byte in temp]
            temp[0] ^= rcon
            rcon = _xtime(rcon)
        words.append([a ^ b for a, b in zip(words[index - 4], temp)])
    rounds = []
    for round_index in range(11):
        block = []
        for word in words[round_index * 4:round_index * 4 + 4]:
            block.extend(word)
        rounds.append(block)
    return rounds


def _inv_shift(state):
    out = state[:]
    for row in range(1, 4):
        line = [state[row + 4 * col] for col in range(4)]
        line = line[-row:] + line[:-row]
        for col in range(4):
            out[row + 4 * col] = line[col]
    return out


def _inv_mix(state):
    out = [0] * 16
    for col in range(4):
        column = state[4 * col:4 * col + 4]
        out[4 * col + 0] = _mul(column[0], 14) ^ _mul(column[1], 11) ^ _mul(column[2], 13) ^ _mul(column[3], 9)
        out[4 * col + 1] = _mul(column[0], 9) ^ _mul(column[1], 14) ^ _mul(column[2], 11) ^ _mul(column[3], 13)
        out[4 * col + 2] = _mul(column[0], 13) ^ _mul(column[1], 9) ^ _mul(column[2], 14) ^ _mul(column[3], 11)
        out[4 * col + 3] = _mul(column[0], 11) ^ _mul(column[1], 13) ^ _mul(column[2], 9) ^ _mul(column[3], 14)
    return out


def _decrypt_block(block, rounds):
    state = [a ^ b for a, b in zip(block, rounds[10])]
    for round_index in range(9, 0, -1):
        state = _inv_shift(state)
        state = [_INV_SBOX[byte] for byte in state]
        state = [a ^ b for a, b in zip(state, rounds[round_index])]
        state = _inv_mix(state)
    state = _inv_shift(state)
    state = [_INV_SBOX[byte] for byte in state]
    return [a ^ b for a, b in zip(state, rounds[0])]


def aes128_cbc_decrypt(data, key, iv):
    rounds = _expand_key(key)
    previous = iv
    plain = bytearray()
    for offset in range(0, len(data), 16):
        cipher = data[offset:offset + 16]
        block = _decrypt_block(cipher, rounds)
        plain.extend(a ^ b for a, b in zip(block, previous))
        previous = cipher
    return bytes(plain)


def load_aes128():
    key = None
    iv = None
    if not os.path.isfile(KEY_FILE):
        return None
    for raw in open(KEY_FILE, "r", encoding="utf-8-sig"):
        line = raw.strip().lstrip("\ufeff")
        if (not line) or line.startswith("#"):
            continue
        parts = line.split(None, 1)
        if len(parts) != 2:
            continue
        name = parts[0].upper()
        if name not in ("KEY", "IV", "密钥", "向量"):
            continue
        try:
            blob = bytes.fromhex(parts[1].replace(" ", ""))
        except ValueError:
            return None
        if name in ("KEY", "密钥"):
            key = blob
        else:
            iv = blob
    if key is None or iv is None or len(key) != 16 or len(iv) != 16:
        return None
    return key, iv


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
    names = {
        0x01: "确认",
        0x02: "拒绝",
        0x60: "标志记录",
        0x61: "升级查询回复",
        0x80: "心跳",
        0x81: "控制",
        0xE0: "查询",
        0xE1: "开始",
        0xE2: "数据",
        0xE3: "结束",
        0xE4: "确认试运行",
        0xE5: "放弃升级",
    }
    if kind == CMD_INFO_RSP and len(body) == 13:
        run, version, st_a, st_b, trial, count, addr = struct.unpack("<BIBBBBI", body)
        return ("当前正在跑 %s 面，版本 %u。A 面%s，B 面%s。试运行面 %s，已试 %u 次。下一写入地址 %08X。"
                % (slot_name(run), version,
                   STATE_NAME.get(st_a, str(st_a)), STATE_NAME.get(st_b, str(st_b)),
                   slot_name(trial), count, addr))
    if kind in (CMD_ACK, CMD_NACK) and len(body) >= 2:
        ask = names.get(body[0], "指令 %02X" % body[0])
        result = RES_NAME.get(body[1], "结果 %u" % body[1])
        word = "同意" if kind == CMD_ACK else "拒绝"
        extra = ""
        if kind == CMD_NACK and len(body) >= 6:
            extra = "，设备要的地址是 %08X" % struct.unpack_from("<I", body, 2)[0]
        return "%s「%s」：%s%s" % (word, ask, result, extra)
    if kind == 0x60 and len(body) >= 25:
        seq = struct.unpack_from("<I", body, 0)[0]
        return ("标志记录序号 %u。记录里要跑 %s，试运行 %s，次数 %u。A 面%s，B 面%s。"
                % (seq, slot_name(body[4]), slot_name(body[5]), body[6],
                   STATE_NAME.get(body[8], str(body[8])),
                   STATE_NAME.get(body[24], str(body[24]))))
    return names.get(kind, "未识别指令 %02X" % kind)


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
            self.log("含义 " + explain(frame))
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
        self.root.title("升级功能测试-加密")
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
        ttk.Label(mid, text="HEX").pack(side=tk.LEFT)
        self.path = tk.StringVar()
        ttk.Entry(mid, textvariable=self.path, width=56).pack(side=tk.LEFT, padx=4)
        ttk.Button(mid, text="选择", command=self.browse).pack(side=tk.LEFT)

        buttons = ttk.Frame(root, padding=(8, 0, 8, 8))
        buttons.pack(fill=tk.X)
        ttk.Button(buttons, text="查询", command=lambda: self.run(self.do_info)).pack(side=tk.LEFT)
        ttk.Button(buttons, text="升级", command=lambda: self.run(self.do_upgrade)).pack(side=tk.LEFT, padx=4)
        ttk.Button(buttons, text="确认", command=lambda: self.run(self.do_confirm)).pack(side=tk.LEFT)
        ttk.Button(buttons, text="放弃", command=lambda: self.run(self.do_abort)).pack(side=tk.LEFT, padx=4)

        self.text = scrolledtext.ScrolledText(root, height=24, font=("Consolas", 10))
        self.text.pack(fill=tk.BOTH, expand=True, padx=8, pady=(0, 8))
        self.log("日志文件：%s" % self.log_path)
        self.log("请选芯片的 USB 串口（1A86:FE0C）。HEX 用合并烧录文件，工具只发送里面的 A 面和 B 面。")
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
        path = filedialog.askopenfilename(filetypes=[("hex", "*.hex"), ("全部", "*.*")])
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
                extra = "，期望地址 %08X" % struct.unpack_from("<I", body, 2)[0]
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
            self.log("查询回复不是 13 字节的升级状态")
            return
        self.log("查询完成。上面一行含义就是设备现在的状态，查询不会改记录，也不用复位。")

    def load_hex(self):
        path = self.path.get().strip()
        if not path:
            self.log("还没有选择 HEX")
            return None
        mem = {}
        upper = 0
        for line_no, raw in enumerate(open(path, "r", encoding="ascii", errors="replace"), 1):
            line = raw.strip()
            if not line:
                continue
            if not line.startswith(":"):
                self.log("第 %u 行不是 HEX 记录" % line_no)
                return None
            try:
                rec = bytes.fromhex(line[1:])
            except ValueError:
                self.log("第 %u 行有非十六进制字符" % line_no)
                return None
            if len(rec) < 5 or rec[0] != len(rec) - 5:
                self.log("第 %u 行长度不对" % line_no)
                return None
            if ((sum(rec[:-1]) + rec[-1]) & 0xFF) != 0:
                self.log("第 %u 行校验和不对" % line_no)
                return None
            count = rec[0]
            offset = (rec[1] << 8) | rec[2]
            kind = rec[3]
            if kind == 0:
                base = upper + offset
                for i in range(count):
                    addr = base + i
                    if addr < 0x10000:
                        addr += 0x08000000
                    mem[addr] = rec[4 + i]
            elif kind == 1:
                break
            elif kind == 2 and count == 2:
                upper = ((rec[4] << 8) | rec[5]) << 4
            elif kind == 4 and count == 2:
                upper = ((rec[4] << 8) | rec[5]) << 16
            elif kind == 5:
                continue
            else:
                self.log("第 %u 行记录类型 %u 不认识" % (line_no, kind))
                return None
        material = load_aes128()
        if material is None:
            self.log("密钥.txt 不是 16 字节密钥加 16 字节向量，和固件对不上")
            return None
        key, iv = material
        faces = []
        for base, name in ((SLOT_A, "A"), (SLOT_B, "B")):
            cipher = bytearray(SLOT_LEN)
            present = 0
            for i in range(SLOT_LEN):
                addr = base + i
                if addr in mem:
                    cipher[i] = mem[addr]
                    present += 1
            if present != SLOT_LEN:
                self.log("%s 面密文不是 %u 字节，不发送" % (name, SLOT_LEN))
                return None
            plain = aes128_cbc_decrypt(bytes(cipher), key, iv)
            magic, link, version, img_end = struct.unpack_from("<IIII", plain, HDR_OFS)
            self.log("%s 面密文 %u 字节。解开后头 magic=%08X link=%X version=%u img_end=%X" % (
                name, SLOT_LEN, magic, link, version, img_end))
            if magic != IMG_MAGIC or link != LINK_OFS[base]:
                self.log("%s 面解开后不是这一面，不发送" % name)
                return None
            crc = zlib.crc32(plain) & 0xFFFFFFFF
            self.log("%s 面明文 CRC32 %08X，发出去的是密文" % (name, crc))
            faces.append((base, bytes(cipher), crc))
        boot = sum(1 for addr in mem if addr < SLOT_A)
        self.log("Boot 区有 %u 字节，升级不发送" % boot)
        return faces

    def do_upgrade(self):
        self.log("—— 升级 ——")
        faces = self.load_hex()
        if faces is None:
            return
        self.log("整份按地址发送密文。设备正在跑的那一面直接回成功，只把另一面解开后写入。")
        start = struct.pack("<IIII", SLOT_LEN, faces[0][2], SLOT_LEN, faces[1][2])
        if self.need_ack(CMD_START, start, 3.0, "开始") is None:
            return
        chunks = []
        for base, blob, _crc in faces:
            for off in range(0, SLOT_LEN, DATA_MAX):
                chunks.append((base + off, blob[off:off + DATA_MAX]))
        index = 0
        retries = 0
        sent = 0
        while index < len(chunks):
            addr, chunk = chunks[index]
            kind, body = self.port.transact(CMD_DATA, struct.pack("<I", addr) + chunk, 3.0)
            if kind == CMD_ACK and len(body) >= 2 and body[1] == 0:
                index += 1
                sent += len(chunk)
                retries = 0
                if index == 1 or sent == SLOT_LEN or sent == SLOT_LEN * 2 or (sent % (DATA_MAX * 20)) == 0:
                    self.log("已发送到地址 %08X，累计 %u / %u" % (addr, sent, SLOT_LEN * 2))
                continue
            expect = None
            if kind == CMD_NACK and len(body) >= 6:
                expect = struct.unpack_from("<I", body, 2)[0]
                self.log("地址 %08X 被拒绝，设备要的地址是 %08X" % (addr, expect))
            else:
                self.log("地址 %08X 没有成功" % addr)
            retries += 1
            if retries > 5:
                self.log("同一位置连续失败，升级停止。停在地址 %08X" % addr)
                return
            if expect is not None:
                found = None
                for i, item in enumerate(chunks):
                    if item[0] == expect:
                        found = i
                        break
                if found is None:
                    self.log("设备要的地址不在这份 A/B 数据里")
                    return
                index = found
                sent = found * DATA_MAX
            self.log("从地址 %08X 再发，第 %u 次" % (chunks[index][0], retries))
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
