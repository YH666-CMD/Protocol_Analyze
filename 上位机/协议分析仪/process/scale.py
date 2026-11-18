# -*- coding: utf-8 -*-
"""探头前端是 11 倍衰减。换算只在上位机做。"""

VREF = 3.3
ADC_FULL = 4095.0
PROBE = 11.0


def adc_volts(count):
    if count < 0:
        count = 0
    if count > 4095:
        count = 4095
    return (count * VREF / ADC_FULL) * PROBE


def pwm_real(value):
    return value * PROBE
