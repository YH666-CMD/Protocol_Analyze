# -*- coding: utf-8 -*-
"""加密升级包。HEX 里是密文，发出去仍是密文。开始命令里的 CRC 是明文。"""

import os
import struct
import zlib

from protocol import CMD_DATA, CMD_END, CMD_START

SLOT_LEN = 30 * 1024
SLOT_A = 0x08000800
SLOT_B = 0x08008000
HDR_OFS = 0x100
IMG_MAGIC = 0x31474D49
LINK_OFS = {SLOT_A: 0x800, SLOT_B: 0x8000}


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
    previous = list(iv)
    plain = bytearray()
    for offset in range(0, len(data), 16):
        cipher = list(data[offset:offset + 16])
        block = _decrypt_block(cipher, rounds)
        plain.extend(a ^ b for a, b in zip(block, previous))
        previous = cipher
    return bytes(plain)


def load_aes128(path):
    key = None
    iv = None
    if not os.path.isfile(path):
        return None
    for raw in open(path, "r", encoding="utf-8-sig"):
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


def _parse_hex(path, log):
    mem = {}
    upper = 0
    for line_no, raw in enumerate(open(path, "r", encoding="ascii", errors="replace"), 1):
        line = raw.strip()
        if not line:
            continue
        if not line.startswith(":"):
            log("第 %u 行不是 HEX" % line_no)
            return None
        try:
            rec = bytes.fromhex(line[1:])
        except ValueError:
            log("第 %u 行不是十六进制" % line_no)
            return None
        if len(rec) < 5 or rec[0] != len(rec) - 5:
            log("第 %u 行长度不对" % line_no)
            return None
        if ((sum(rec[:-1]) + rec[-1]) & 0xFF) != 0:
            log("第 %u 行校验和不对" % line_no)
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
            log("第 %u 行类型 %u 不认识" % (line_no, kind))
            return None
    return mem


def load_cipher_faces(path, key_path, log):
    mem = _parse_hex(path, log)
    if mem is None:
        return None
    material = load_aes128(key_path)
    if material is None:
        log("找不到 16 字节密钥和向量：%s" % key_path)
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
            log("%s 面不是 %u 字节密文" % (name, SLOT_LEN))
            return None
        plain = aes128_cbc_decrypt(bytes(cipher), key, iv)
        magic, link, version, img_end = struct.unpack_from("<IIII", plain, HDR_OFS)
        log("%s 解开后 magic=%08X link=%X version=%u" % (name, magic, link, version))
        if magic != IMG_MAGIC or link != LINK_OFS[base]:
            log("%s 面解开后不是这一面" % name)
            return None
        crc = zlib.crc32(plain) & 0xFFFFFFFF
        log("%s 明文 CRC32 %08X，发出去的是密文" % (name, crc))
        faces.append((base, bytes(cipher), crc))
    return faces


def send_upgrade(link, faces, log, need_ack):
    import struct
    log("按地址发送密文。正在跑的那一面只应答，另一面解开后写入。")
    start = struct.pack("<IIII", SLOT_LEN, faces[0][2], SLOT_LEN, faces[1][2])
    if need_ack(CMD_START, start, 3.0, "开始") is None:
        return False
    chunks = []
    for base, blob, _crc in faces:
        for off in range(0, SLOT_LEN, 48):
            chunks.append((base + off, blob[off:off + 48]))
    index = 0
    retries = 0
    sent = 0
    while index < len(chunks):
        addr, chunk = chunks[index]
        kind, body = link.transact(CMD_DATA, struct.pack("<I", addr) + chunk, 3.0)
        if kind == 0x01 and len(body) >= 2 and body[1] == 0:
            index += 1
            sent += len(chunk)
            retries = 0
            if index == 1 or sent in (SLOT_LEN, SLOT_LEN * 2) or (sent % (48 * 20)) == 0:
                log("已到 %08X，累计 %u / %u" % (addr, sent, SLOT_LEN * 2))
            continue
        expect = None
        if kind == 0x02 and len(body) >= 6:
            expect = struct.unpack_from("<I", body, 2)[0]
            log("地址 %08X 被拒绝，设备要 %08X" % (addr, expect))
        else:
            log("地址 %08X 没有成功" % addr)
        retries += 1
        if retries > 5:
            log("连续失败，停在 %08X" % addr)
            return False
        if expect is not None:
            found = None
            for i, item in enumerate(chunks):
                if item[0] == expect:
                    found = i
                    break
            if found is None:
                log("设备要的地址不在这份包里")
                return False
            index = found
    if need_ack(CMD_END, b"", 8.0, "结束") is None:
        log("结束没有读到应答。设备复位时 USB 会掉线，重新连接后再查询。")
        return False
    log("结束已应答。新面大约 1 秒后自己留下。不要再发确认。")
    return True
