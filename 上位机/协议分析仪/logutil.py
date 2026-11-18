# -*- coding: utf-8 -*-
"""每个通道一个目录、一次启动一个 txt。"""

import datetime
import os
import queue


def app_dir():
    import sys
    if getattr(sys, "frozen", False):
        return os.path.dirname(sys.executable)
    return os.path.dirname(os.path.abspath(__file__))


class ChannelLog(object):
    def __init__(self, folder, title):
        os.makedirs(folder, exist_ok=True)
        stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        self.path = os.path.join(folder, "%s_%s.txt" % (title, stamp))
        self.file = open(self.path, "a", encoding="utf-8")
        self.lines = queue.Queue()

    def write(self, text):
        line = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3] + "  " + text
        self.lines.put(line)
        self.file.write(line + "\n")
        self.file.flush()
        return line

    def close(self):
        try:
            self.file.close()
        except Exception:
            pass
