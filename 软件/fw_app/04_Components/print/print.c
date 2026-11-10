#include "print.h"
#include "config.h"
#include <stdarg.h>
#include <stddef.h>

static print_putc_fn s_putc;

static result_t print_init(print_putc_fn putc)
{
#if (PRINT_ENABLE == 0u)
    (void)putc;
    s_putc = NULL;
    return RES_OK;
#else
    if (putc == NULL)
    {
        return RES_PARAM;
    }
    s_putc = putc;
    return RES_OK;
#endif
}

#if (PRINT_ENABLE != 0u)
static result_t print_out(uint8_t ch)
{
    if (s_putc == NULL)
    {
        return RES_STATE;
    }
    return s_putc(ch);
}

static result_t print_repeat(char ch, uint8_t count)
{
    result_t rc;

    while (count > 0u)
    {
        rc = print_out((uint8_t)ch);
        if (rc != RES_OK)
        {
            return rc;
        }
        count--;
    }
    return RES_OK;
}

/* 数字先倒着放进 buf，再按宽度补齐后正序送出。base 只接受 10 或 16。 */
static result_t print_num(uint32_t value, uint8_t base, uint8_t width, char pad, uint8_t upper)
{
    char buf[10];
    uint8_t n;
    result_t rc;
    uint8_t digit;

    n = 0u;
    do
    {
        digit = (uint8_t)(value % base);
        if (digit < 10u)
        {
            buf[n] = (char)('0' + digit);
        }
        else if (upper != 0u)
        {
            buf[n] = (char)('A' + (digit - 10u));
        }
        else
        {
            buf[n] = (char)('a' + (digit - 10u));
        }
        n++;
        value /= base;
    } while (value != 0u && n < (uint8_t)sizeof(buf));

    if (width > (uint8_t)sizeof(buf))
    {
        width = (uint8_t)sizeof(buf);
    }
    if (width > n)
    {
        rc = print_repeat(pad, (uint8_t)(width - n));
        if (rc != RES_OK)
        {
            return rc;
        }
    }

    while (n > 0u)
    {
        n--;
        rc = print_out((uint8_t)buf[n]);
        if (rc != RES_OK)
        {
            return rc;
        }
    }
    return RES_OK;
}

static result_t print_str(const char *text)
{
    result_t rc;

    if (text == NULL)
    {
        text = "(null)";
    }
    while (*text != '\0')
    {
        rc = print_out((uint8_t)*text);
        if (rc != RES_OK)
        {
            return rc;
        }
        text++;
    }
    return RES_OK;
}

static result_t print_format(const char *fmt, va_list ap)
{
    result_t rc;
    char pad;
    uint8_t width;
    int32_t signed_value;

    rc = RES_OK;
    while (*fmt != '\0')
    {
        if (*fmt != '%')
        {
            rc = print_out((uint8_t)*fmt);
            fmt++;
            if (rc != RES_OK)
            {
                return rc;
            }
            continue;
        }

        fmt++;
        if (*fmt == '\0')
        {
            return rc;
        }
        if (*fmt == '%')
        {
            rc = print_out((uint8_t)'%');
            fmt++;
            if (rc != RES_OK)
            {
                return rc;
            }
            continue;
        }

        pad = ' ';
        width = 0u;
        if (*fmt == '0')
        {
            pad = '0';
            fmt++;
        }
        while (*fmt >= '0' && *fmt <= '9')
        {
            width = (uint8_t)((width * 10u) + (uint8_t)(*fmt - '0'));
            if (width > 10u)
            {
                width = 10u;
            }
            fmt++;
        }

        switch (*fmt)
        {
        case 'c':
            rc = print_out((uint8_t)va_arg(ap, int));
            break;
        case 's':
            rc = print_str(va_arg(ap, const char *));
            break;
        case 'd':
            signed_value = va_arg(ap, int32_t);
            if (signed_value < 0)
            {
                rc = print_out((uint8_t)'-');
                if (rc != RES_OK)
                {
                    return rc;
                }
                rc = print_num(0u - (uint32_t)signed_value, 10u, width, pad, 0u);
            }
            else
            {
                rc = print_num((uint32_t)signed_value, 10u, width, pad, 0u);
            }
            break;
        case 'u':
            rc = print_num(va_arg(ap, uint32_t), 10u, width, pad, 0u);
            break;
        case 'x':
            rc = print_num(va_arg(ap, uint32_t), 16u, width, pad, 0u);
            break;
        case 'X':
            rc = print_num(va_arg(ap, uint32_t), 16u, width, pad, 1u);
            break;
        default:
            rc = print_out((uint8_t)*fmt);
            break;
        }

        if (*fmt != '\0')
        {
            fmt++;
        }
        if (rc != RES_OK)
        {
            return rc;
        }
    }
    return RES_OK;
}
#endif

static result_t print_write(const uint8_t *data, uint16_t len)
{
#if (PRINT_ENABLE == 0u)
    (void)data;
    (void)len;
    return RES_OK;
#else
    uint16_t i;
    result_t rc;

    if (data == NULL && len != 0u)
    {
        return RES_PARAM;
    }
    if (s_putc == NULL)
    {
        return RES_STATE;
    }
    for (i = 0u; i < len; i++)
    {
        rc = print_out(data[i]);
        if (rc != RES_OK)
        {
            return rc;
        }
    }
    return RES_OK;
#endif
}

static result_t print_printf(const char *fmt, ...)
{
    if (fmt == NULL)
    {
        return RES_PARAM;
    }
#if (PRINT_ENABLE == 0u)
    return RES_OK;
#else
    va_list ap;
    result_t rc;

    if (s_putc == NULL)
    {
        return RES_STATE;
    }
    va_start(ap, fmt);
    rc = print_format(fmt, ap);
    va_end(ap);
    return rc;
#endif
}

const print_driver_t print_sw_driver =
{
    .pf_init   = print_init,
    .pf_write  = print_write,
    .pf_printf = print_printf,
};
