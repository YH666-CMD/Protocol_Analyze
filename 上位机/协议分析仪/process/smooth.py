# -*- coding: utf-8 -*-
"""显示用平滑。不改已经写进日志的原始样本。"""


def ema_step(prev, sample, alpha):
    if prev is None:
        return float(sample)
    return alpha * float(sample) + (1.0 - alpha) * prev


def linear_resample(values, count):
    """把一条折线拉成 count 个点，点与点之间线性插值。"""
    n = len(values)
    if n == 0 or count <= 1:
        return list(values)
    if n == 1:
        return [values[0]] * count
    out = []
    last = n - 1
    for i in range(count):
        pos = i * last / float(count - 1)
        i0 = int(pos)
        i1 = i0 + 1 if i0 < last else last
        frac = pos - i0
        out.append(values[i0] * (1.0 - frac) + values[i1] * frac)
    return out
