# -*- coding: utf-8 -*-
"""ADC 波形视图。时间轴按板子现有采样率估算，不改下位机。

交互：
  Auto    按当前勾选的通道，把纵轴和横轴收到能看清的范围。
  满量程  纵轴回到 0 到满量程，横轴大约 50 ms，并跟上最新样本。
  最新    横轴右端回到最新样本。
  纵轴按钮、滚轮          放大缩小或上下移动。
  横轴按钮、Shift+滚轮    放大缩小或左右移动。
  拖动    抓住波形平移。离开最新样本后不再自动往前滚。
"""

import math
import tkinter as tk
from tkinter import ttk

from process.scale import ADC_FULL, adc_volts

COLORS = ("#4da3ff", "#ff5d5d", "#3ddc84")
NAMES = ("CH0  PA0", "CH1  PA1", "CH2  PA2")

# PCLK2 96 MHz、ADC 8 分频、采样 239.5 再加转换 12.5。三路轮流时每路约 63 微秒。
ADC_HZ = 12e6
ADC_CYCLES = 239.5 + 12.5

X_DIV = 10
Y_DIV = 8
ZOOM_V = 1.6
ZOOM_H = 1.8
PAN_V = 0.18
PAN_H = 0.25
DEFAULT_SPAN = 800
MIN_SPAN = 16


def sample_us(nch):
    if nch < 1:
        nch = 1
    return (ADC_CYCLES / ADC_HZ) * nch * 1e6


def full_scale(volts):
    if volts:
        return adc_volts(ADC_FULL)
    return float(ADC_FULL)


def nice_span(span):
    """把一段范围收到 1、2、5 乘十的幂，并且不小于原来的范围。"""
    if span <= 0:
        return 1.0
    exp = math.floor(math.log10(span))
    base = span / (10 ** exp)
    if base <= 1.0:
        nice = 1.0
    elif base <= 2.0:
        nice = 2.0
    elif base <= 5.0:
        nice = 5.0
    else:
        nice = 10.0
    return nice * (10 ** exp)


def period_samples(values):
    """用带回差的上升沿估一个周期。不稳就返回 None。"""
    n = len(values)
    if n < 16:
        return None
    lo = min(values)
    hi = max(values)
    pp = hi - lo
    if pp <= 0:
        return None
    mid = (lo + hi) * 0.5
    hyst = pp * 0.2
    low = mid - hyst
    high = mid + hyst
    armed = False
    marks = []
    for index, value in enumerate(values):
        if not armed:
            if value <= low:
                armed = True
        elif value >= high:
            marks.append(index)
            armed = False
    if len(marks) < 3:
        return None
    gaps = [marks[i + 1] - marks[i] for i in range(len(marks) - 1)]
    gaps.sort()
    med = gaps[len(gaps) // 2]
    if med < 4:
        return None
    if gaps[-1] - gaps[0] > med * 0.4:
        return None
    return float(med)


def window_list(seq, start, end):
    """取出 [start, end)。deque 不能切片，列表直接切。"""
    n = len(seq)
    if n == 0 or end <= 0 or start >= n:
        return []
    if start < 0:
        start = 0
    if end > n:
        end = n
    if start >= end:
        return []
    if isinstance(seq, list):
        return seq[start:end]
    out = []
    for index, value in enumerate(seq):
        if index >= end:
            break
        if index >= start:
            out.append(value)
    return out


def fmt_value(value, volts):
    if volts:
        if abs(value) >= 10:
            return "%.2f V" % value
        return "%.3f V" % value
    return "%d" % int(round(value))


def fmt_span(value, volts):
    if volts:
        if value >= 1:
            return "%.2f V" % value
        if value >= 0.01:
            return "%.3f V" % value
        return "%.1f mV" % (value * 1000.0)
    return "%d" % int(round(value))


def fmt_time(us):
    if us >= 1e6:
        return "%.2f s" % (us / 1e6)
    if us >= 1000:
        return "%.2f ms" % (us / 1000.0)
    if us >= 1:
        return "%.0f us" % us
    return "%.1f us" % us


def fmt_hz(hz):
    if hz >= 1000:
        return "%.2f kHz" % (hz / 1000.0)
    if hz >= 10:
        return "%.1f Hz" % hz
    return "%.2f Hz" % hz


class AdcScope(ttk.Frame):
    def __init__(self, parent):
        ttk.Frame.__init__(self, parent)
        self.y0 = 0.0
        self.y1 = full_scale(True)
        self.span = DEFAULT_SPAN
        self.lag = 0
        self.volts = True
        self.dt_us = sample_us(3)
        self.series = [[], [], []]
        self._shown = [[], [], []]
        self.show = [tk.BooleanVar(value=True) for _ in range(3)]
        self.cursor = None
        self._drag = None

        bar = ttk.Frame(self)
        bar.pack(fill=tk.X, padx=4, pady=(0, 2))
        ttk.Button(bar, text="Auto", width=6, command=self.auto).pack(side=tk.LEFT)
        ttk.Button(bar, text="满量程", width=7, command=self.full).pack(side=tk.LEFT, padx=4)
        ttk.Button(bar, text="最新", width=5, command=self.latest).pack(side=tk.LEFT)
        ttk.Label(bar, text="纵轴").pack(side=tk.LEFT, padx=(10, 2))
        ttk.Button(bar, text="放大", width=4, command=lambda: self.zoom_y(ZOOM_V)).pack(side=tk.LEFT)
        ttk.Button(bar, text="缩小", width=4, command=lambda: self.zoom_y(1.0 / ZOOM_V)).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="上", width=3, command=lambda: self.pan_y(-1)).pack(side=tk.LEFT)
        ttk.Button(bar, text="下", width=3, command=lambda: self.pan_y(1)).pack(side=tk.LEFT, padx=(2, 0))
        bar2 = ttk.Frame(self)
        bar2.pack(fill=tk.X, padx=4, pady=(0, 2))
        ttk.Label(bar2, text="横轴").pack(side=tk.LEFT, padx=(0, 2))
        ttk.Button(bar2, text="放大", width=4, command=lambda: self.zoom_x(ZOOM_H)).pack(side=tk.LEFT)
        ttk.Button(bar2, text="缩小", width=4, command=lambda: self.zoom_x(1.0 / ZOOM_H)).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar2, text="左", width=3, command=lambda: self.pan_x(1)).pack(side=tk.LEFT)
        ttk.Button(bar2, text="右", width=3, command=lambda: self.pan_x(-1)).pack(side=tk.LEFT, padx=(2, 8))
        self.scale_lab = ttk.Label(bar2, text="")
        self.scale_lab.pack(side=tk.LEFT)

        self.canvas = tk.Canvas(self, height=260, bg="#12151c", highlightthickness=0)
        self.canvas.bind("<Configure>", lambda _e: self.redraw())
        self.canvas.bind("<MouseWheel>", self._wheel)
        self.canvas.bind("<Button-4>", lambda e: self._wheel_dir(e, 1))
        self.canvas.bind("<Button-5>", lambda e: self._wheel_dir(e, -1))
        self.canvas.bind("<Enter>", lambda _e: self.canvas.focus_set())
        self.canvas.bind("<Motion>", self._motion)
        self.canvas.bind("<Leave>", self._leave)
        self.canvas.bind("<ButtonPress-1>", self._press)
        self.canvas.bind("<B1-Motion>", self._drag_move)
        self.canvas.bind("<ButtonRelease-1>", self._release)

        info = ttk.Frame(self)
        self.info_labs = []
        for index in range(3):
            row = ttk.Frame(info)
            row.pack(fill=tk.X)
            ttk.Checkbutton(row, variable=self.show[index], command=self.redraw).pack(side=tk.LEFT)
            tk.Label(row, text=NAMES[index], fg=COLORS[index], width=10, anchor="w").pack(side=tk.LEFT)
            lab = tk.Label(row, text="还没有数据", anchor="w", fg="#222")
            lab.pack(side=tk.LEFT, fill=tk.X, expand=True)
            self.info_labs.append(lab)

        hint = ttk.Label(self, text="滚轮缩放纵轴，按住 Shift 缩放横轴，拖动平移。不看的通道去掉勾再按 Auto。")
        hint.pack(side=tk.BOTTOM, anchor="w", padx=6, pady=(0, 2))
        info.pack(side=tk.BOTTOM, fill=tk.X, padx=4, pady=(0, 2))
        self.canvas.pack(fill=tk.BOTH, expand=True, padx=4, pady=2)

    def draw(self, series, volts, dt_us):
        self.series = series
        if self.volts != volts:
            self._convert_axis(volts)
        self.volts = volts
        if dt_us > 0:
            self.dt_us = dt_us
        self._shown = self._display_series()
        self._clamp()
        self.redraw()

    def redraw(self):
        if not self.canvas.winfo_exists():
            return
        self._paint()
        self._paint_info()

    def auto(self):
        series = self._display_series()
        picked = self._checked(series)
        if not picked:
            return
        best = None
        best_pp = -1.0
        for values in picked:
            if len(values) < 8:
                continue
            sample = values[-min(len(values), 4096):]
            pp = max(sample) - min(sample)
            if pp > best_pp:
                best_pp = pp
                best = sample
        if best is None:
            return
        period = period_samples(best)
        if period is not None:
            span = int(round(period * 4.0))
        else:
            span = min(len(best), DEFAULT_SPAN)
        span = max(MIN_SPAN, min(span, max(len(best), MIN_SPAN)))
        self.span = span
        self.lag = 0
        view = self._view_values(picked)
        self._fit_y(view)
        self._clamp()
        self.redraw()

    def full(self):
        self.y0 = 0.0
        self.y1 = full_scale(self.volts)
        self.span = DEFAULT_SPAN
        self.lag = 0
        self._clamp()
        self.redraw()

    def latest(self):
        self.lag = 0
        self.redraw()

    def zoom_y(self, factor, anchor=None):
        span = (self.y1 - self.y0) / factor
        span = self._limit_y_span(span)
        if anchor is None:
            anchor = (self.y0 + self.y1) * 0.5
            frac = 0.5
        else:
            old = self.y1 - self.y0
            frac = 0.5 if old <= 0 else (self.y1 - anchor) / old
        self.y1 = anchor + frac * span
        self.y0 = self.y1 - span
        self._clamp()
        self.redraw()

    def zoom_x(self, factor, anchor_frac=None):
        old = self.span
        new = int(round(old / factor))
        new = max(MIN_SPAN, new)
        if anchor_frac is None:
            anchor_frac = 1.0 if self.lag == 0 else 0.5
        grow = new - old
        self.span = new
        self.lag = self.lag - int(round(grow * (1.0 - anchor_frac)))
        self._clamp()
        self.redraw()

    def pan_y(self, direction):
        shift = (self.y1 - self.y0) * PAN_V * direction
        self.y0 += shift
        self.y1 += shift
        self._clamp()
        self.redraw()

    def pan_x(self, direction):
        step = max(1, int(round(self.span * PAN_H)))
        self.lag += step * direction
        self._clamp()
        self.redraw()

    def _convert_axis(self, volts):
        scale = adc_volts(1.0)
        if scale <= 0:
            return
        if volts:
            self.y0 *= scale
            self.y1 *= scale
        else:
            self.y0 /= scale
            self.y1 /= scale

    def _display_series(self):
        out = []
        for values in self.series:
            if self.volts:
                out.append([adc_volts(v) for v in values])
            else:
                out.append([float(v) for v in values])
        return out

    def _checked(self, series):
        picked = []
        for index, values in enumerate(series):
            if self.show[index].get() and len(values) > 0:
                picked.append(values)
        return picked

    def _edges(self, length):
        if length <= 0:
            return 0, 0
        right = length - 1 - self.lag
        if right >= length:
            right = length - 1
        left = right - (self.span - 1)
        return left, right

    def _view_values(self, series):
        chunks = []
        for values in series:
            left, right = self._edges(len(values))
            chunks.extend(window_list(values, left, right + 1))
        return chunks

    def _fit_y(self, values):
        if not values:
            return
        lo = min(values)
        hi = max(values)
        floor = full_scale(self.volts) * 0.004
        if hi - lo < floor:
            mid = (lo + hi) * 0.5
            lo = mid - floor * 0.5
            hi = mid + floor * 0.5
        need = (hi - lo) * 1.22
        per = nice_span(need / float(Y_DIV))
        span = per * Y_DIV
        if span > need * 1.55:
            span = need
        mid = (lo + hi) * 0.5
        self.y0 = mid - span * 0.5
        self.y1 = mid + span * 0.5

    def _limit_y_span(self, span):
        floor = full_scale(self.volts) * 0.002
        ceil = full_scale(self.volts) * 1.6
        if span < floor:
            return floor
        if span > ceil:
            return ceil
        return span

    def _clamp(self):
        span = self._limit_y_span(self.y1 - self.y0)
        mid = (self.y0 + self.y1) * 0.5
        self.y0 = mid - span * 0.5
        self.y1 = mid + span * 0.5
        full = full_scale(self.volts)
        if self.y0 < -0.25 * full:
            self.y1 -= self.y0 + 0.25 * full
            self.y0 = -0.25 * full
        if self.y1 > 1.25 * full:
            self.y0 -= self.y1 - 1.25 * full
            self.y1 = 1.25 * full
        longest = 0
        for values in self.series:
            if len(values) > longest:
                longest = len(values)
        if self.span < MIN_SPAN:
            self.span = MIN_SPAN
        if self.span > 8192:
            self.span = 8192
        if self.lag < 0:
            self.lag = 0
        if longest > 0 and self.lag > longest - 1:
            self.lag = longest - 1

    def _plot_box(self):
        width = max(self.canvas.winfo_width(), 10)
        height = max(self.canvas.winfo_height(), 10)
        left, top, right, bottom = 64, 18, width - 12, height - 22
        if right - left < 20 or bottom - top < 20:
            return None
        return left, top, right, bottom

    def _y_pix(self, value, top, bottom):
        span = self.y1 - self.y0
        if span == 0:
            return (top + bottom) * 0.5
        pos = top + (self.y1 - value) / span * (bottom - top)
        if pos < top:
            return top
        if pos > bottom:
            return bottom
        return pos

    def _paint(self):
        canvas = self.canvas
        canvas.delete("all")
        box = self._plot_box()
        if box is None:
            return
        left, top, right, bottom = box
        self._grid(canvas, left, top, right, bottom)
        series = self._shown
        any_on = False
        for index, values in enumerate(series):
            if not self.show[index].get() or len(values) < 2:
                continue
            any_on = True
            self._trace(canvas, values, COLORS[index], left, top, right, bottom)
        if not any_on:
            canvas.create_text((left + right) / 2, (top + bottom) / 2,
                               text="还没有波形", fill="#8b93a7")
        self._cursor(canvas, series, left, top, right, bottom)
        v_div = (self.y1 - self.y0) / Y_DIV
        t_div = self.span * self.dt_us / X_DIV
        follow = "跟踪最新" if self.lag == 0 else "已离开最新"
        vtext = fmt_span(v_div, self.volts)
        if not self.volts:
            vtext += " 码"
        self.scale_lab["text"] = "纵轴 %s/格    横轴 %s/格    %s" % (
            vtext, fmt_time(t_div), follow)

    def _grid(self, canvas, left, top, right, bottom):
        for step in range(Y_DIV + 1):
            y = top + (bottom - top) * step / Y_DIV
            canvas.create_line(left, y, right, y, fill="#2a3142")
            value = self.y1 - (self.y1 - self.y0) * step / Y_DIV
            canvas.create_text(left - 6, y, anchor="e", text=self._tick(value), fill="#9aa3b5",
                               font=("Consolas", 8))
        for step in range(X_DIV + 1):
            x = left + (right - left) * step / X_DIV
            canvas.create_line(x, top, x, bottom, fill="#2a3142")
            if step % 2 == 0:
                age = (self.span - 1) * (1.0 - step / float(X_DIV)) + self.lag
                canvas.create_text(x, bottom + 10, anchor="n", text=fmt_time(age * self.dt_us),
                                   fill="#9aa3b5", font=("Consolas", 8))

    def _tick(self, value):
        if self.volts:
            if abs(self.y1 - self.y0) >= 5:
                return "%.1f" % value
            if abs(self.y1 - self.y0) >= 0.5:
                return "%.2f" % value
            return "%.3f" % value
        return "%d" % int(round(value))

    def _trace(self, canvas, values, color, left, top, right, bottom):
        n = len(values)
        view_left, view_right = self._edges(n)
        i0 = view_left if view_left > 0 else 0
        i1 = view_right if view_right < n else n - 1
        if i1 - i0 < 1:
            return
        plot_w = float(right - left)
        count = i1 - i0 + 1
        if count > plot_w * 1.4:
            columns = max(1, int(plot_w))
            coords_hi = []
            coords_lo = []
            for col in range(columns):
                a = i0 + int(col * count / columns)
                b = i0 + int((col + 1) * count / columns)
                if b <= a:
                    b = a + 1
                if b > i1 + 1:
                    b = i1 + 1
                chunk = window_list(values, a, b)
                if not chunk:
                    continue
                x = left + (a - view_left) / float(self.span - 1) * plot_w
                coords_hi.extend((x, self._y_pix(max(chunk), top, bottom)))
                coords_lo.extend((x, self._y_pix(min(chunk), top, bottom)))
            if len(coords_hi) >= 4:
                canvas.create_line(*coords_hi, fill=color, width=1)
                canvas.create_line(*coords_lo, fill=color, width=1)
            return
        coords = []
        for index in range(i0, i1 + 1):
            x = left + (index - view_left) / float(max(1, self.span - 1)) * plot_w
            coords.extend((x, self._y_pix(values[index], top, bottom)))
        if len(coords) >= 4:
            canvas.create_line(*coords, fill=color, width=1.4)

    def _cursor(self, canvas, series, left, top, right, bottom):
        if self.cursor is None:
            return
        x, _y = self.cursor
        if x < left or x > right:
            return
        frac = (x - left) / float(right - left)
        canvas.create_line(x, top, x, bottom, fill="#d0d4dc", dash=(3, 3))
        parts = []
        age = (1.0 - frac) * (self.span - 1) + self.lag
        parts.append(fmt_time(age * self.dt_us))
        for index, values in enumerate(series):
            if not self.show[index].get() or len(values) < 1:
                continue
            view_left, view_right = self._edges(len(values))
            pos = view_left + frac * (self.span - 1)
            sample = int(round(pos))
            if sample < 0 or sample >= len(values):
                continue
            parts.append("%s %s" % (NAMES[index][:3], fmt_value(values[sample], self.volts)))
        canvas.create_text(left + 6, top + 4, anchor="nw", text="  ".join(parts), fill="#e8eaef",
                           font=("Consolas", 9))

    def _paint_info(self):
        series = self._shown
        for index, values in enumerate(series):
            lab = self.info_labs[index]
            if len(values) < 1:
                lab["text"] = "还没有数据"
                continue
            left, right = self._edges(len(values))
            view = window_list(values, left, right + 1)
            if not view:
                view = [values[-1]]
            lo = min(view)
            hi = max(view)
            mean = sum(view) / float(len(view))
            period = period_samples(view)
            if period is None or self.dt_us <= 0:
                freq = "—"
            else:
                freq = fmt_hz(1e6 / (period * self.dt_us))
            lab["text"] = "最新 %s    最小 %s    最大 %s    峰峰 %s    平均 %s    频率 %s" % (
                fmt_value(values[-1], self.volts),
                fmt_value(lo, self.volts),
                fmt_value(hi, self.volts),
                fmt_span(hi - lo, self.volts),
                fmt_value(mean, self.volts),
                freq,
            )

    def _anchor_value(self, event):
        box = self._plot_box()
        if box is None:
            return None
        left, top, right, bottom = box
        y = event.y
        if y < top:
            y = top
        if y > bottom:
            y = bottom
        span = self.y1 - self.y0
        return self.y1 - (y - top) / float(bottom - top) * span

    def _wheel(self, event):
        step = 1 if event.delta > 0 else -1
        self._wheel_dir(event, step)

    def _wheel_dir(self, event, step):
        factor = ZOOM_V if step > 0 else 1.0 / ZOOM_V
        if event.state & 0x0001:
            frac = None
            box = self._plot_box()
            if box is not None:
                left, _top, right, _bottom = box
                frac = (event.x - left) / float(right - left)
                if frac < 0:
                    frac = 0
                if frac > 1:
                    frac = 1
            hfactor = ZOOM_H if step > 0 else 1.0 / ZOOM_H
            self.zoom_x(hfactor, frac)
            return
        self.zoom_y(factor, self._anchor_value(event))

    def _motion(self, event):
        self.cursor = (event.x, event.y)
        self.redraw()

    def _leave(self, _event):
        self.cursor = None
        self._drag = None
        self.redraw()

    def _press(self, event):
        self._drag = (event.x, event.y, self.y0, self.y1, self.lag)

    def _drag_move(self, event):
        if self._drag is None:
            return
        x0, y0, vy0, vy1, lag0 = self._drag
        box = self._plot_box()
        if box is None:
            return
        left, top, right, bottom = box
        dx = event.x - x0
        dy = event.y - y0
        self.y0 = vy0 + dy / float(bottom - top) * (vy1 - vy0)
        self.y1 = vy1 + dy / float(bottom - top) * (vy1 - vy0)
        self.lag = lag0 + int(round(dx / float(right - left) * self.span))
        self._clamp()
        self.cursor = (event.x, event.y)
        self.redraw()

    def _release(self, _event):
        self._drag = None
