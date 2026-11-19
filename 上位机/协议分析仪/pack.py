# -*- coding: utf-8 -*-
"""打成单个 exe。参数放在这里，避免 bat 的编码把命令行拆开。"""

import ctypes
import subprocess
import sys


def console_encoding():
    if sys.platform == "win32":
        code = ctypes.windll.kernel32.GetConsoleOutputCP()
        if code:
            return "cp%d" % code
    return getattr(sys.stdout, "encoding", None) or "gbk"


def say(text):
    data = (text + "\n").encode(console_encoding(), errors="replace")
    sys.stdout.buffer.write(data)
    sys.stdout.buffer.flush()


def main():
    cmd = [
        sys.executable, "-m", "PyInstaller",
        "--noconfirm",
        "--clean",
        "--windowed",
        "--onefile",
        "--name", "协议分析仪",
        "--hidden-import", "serial.tools.list_ports",
        "main.py",
    ]
    code = subprocess.call(cmd)
    if code != 0:
        say("打包失败，上面是 PyInstaller 的报错。")
        return code
    say("生成文件：dist\\协议分析仪.exe")
    say("把 exe 放在本目录旁边运行时，会到上两级的 工具\\加密工具\\密钥.txt 找密钥。")
    say("也可以把 密钥.txt 复制到 exe 旁边。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
