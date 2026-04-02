#include "printk.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <wint_t.h>
#include <string.h>

#define PRINTK_BUFFER_SIZE 64
#define DEFAULT_LEVEL 6
#define MAX_SINKS 8
#define NTOA_BUFFER_SIZE 24

_Static_assert(PRINTK_BUFFER_SIZE >= 1);
_Static_assert(MAX_SINKS >= 1);
_Static_assert(NTOA_BUFFER_SIZE >= 1);

static printk_sink_t sinks[MAX_SINKS];

#define FL_LEFT_JUSTIFY (1 << 0)
#define FL_ALWAYS_SIGN (1 << 1)
#define FL_SIGN_PAD (1 << 2)
#define FL_PREFIX (1 << 3)
#define FL_ZERO_PAD (1 << 4)
#define FL_UPPER (1 << 5) // internal
#define FL_NEG (1 << 6) // internal

typedef enum {
    LEN_NONE,
    LEN_HH,
    LEN_H,
    LEN_L,
    LEN_LL,
    LEN_J,
    LEN_Z,
    LEN_T,
} length_t;

typedef struct {
    char buffer[PRINTK_BUFFER_SIZE];
    int buf_len;
    int level;
    char ntoa[NTOA_BUFFER_SIZE];
    int len;
} printk_context_t;

#define finline inline __attribute__((always_inline))

static finline void _flush(printk_context_t* ctx) {
    if (!ctx || ctx->buf_len == 0) return;
    if (ctx->buf_len > PRINTK_BUFFER_SIZE) ctx->buf_len = PRINTK_BUFFER_SIZE;
    for (int i = 0; i < MAX_SINKS; i++)
        if (sinks[i].name[0]) sinks[i].write(ctx->level, ctx->buffer, ctx->buf_len);
    ctx->buf_len = 0;
}

static finline void _putc(printk_context_t* ctx, char c) {
    if (!ctx) return;
    if (ctx->buf_len >= PRINTK_BUFFER_SIZE) _flush(ctx);
    ctx->buffer[ctx->buf_len++] = c;
    ctx->len++;
}

static finline void _putwc(printk_context_t* ctx, wchar_t wc) {
    if (wc <= 0x7F) {
        _putc(ctx, (char)wc);
    } else if (wc <= 0x7FF) {
        _putc(ctx, 0xC0 | (wc >> 6));
        _putc(ctx, 0x80 | (wc & 0x3F));
    } else if (wc <= 0xFFFF) {
        _putc(ctx, 0xE0 | (wc >> 12));
        _putc(ctx, 0x80 | ((wc >> 6) & 0x3F));
        _putc(ctx, 0x80 | (wc & 0x3F));
    } else if (wc <= 0x10FFFF) {
        _putc(ctx, 0xF0 | (wc >> 18));
        _putc(ctx, 0x80 | ((wc >> 12) & 0x3F));
        _putc(ctx, 0x80 | ((wc >> 6) & 0x3F));
        _putc(ctx, 0x80 | (wc & 0x3F));
    }
}

static finline void _pad(printk_context_t* ctx, char c, int n) {
    for (; n > 0; n--) _putc(ctx, c);
}

static finline void _str(printk_context_t* ctx, const char* s, int len) {
    if (!s) return;
    for (int i = 0; i < len; i++) _putc(ctx, s[i]);
}

static finline void _wstr(printk_context_t* ctx, const wchar_t* s, int len) {
    if (!s) return;
    for (int i = 0; i < len; i++) _putwc(ctx, s[i]);
}

static finline const char* _ntoa(printk_context_t* ctx, uint64_t val, unsigned base, int upper) {
    const char *d = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char *p = ctx->ntoa + NTOA_BUFFER_SIZE;

    if (val == 0) {
        *--p = '0';
        return p;
    }

    while (val && p > ctx->ntoa) {
        *--p = d[val % base];
        val /= base;
    }
    if (val) *p = '?';
    return p;
}

void static finline _integer(printk_context_t* ctx, int is_signed, unsigned base, unsigned flags, int width, int precision, length_t len, va_list va) {
    uint64_t uval = 0;
    if (is_signed) {
        int64_t x = 0;
        switch (len) {
            case LEN_HH: x = (signed char)  va_arg(va, int); break;
            case LEN_H: x = (short int)     va_arg(va, int); break;
            case LEN_L: x =                 va_arg(va, long int); break;
            case LEN_LL: x =                va_arg(va, long long int); break;
            case LEN_J: x =                 va_arg(va, intmax_t); break;
            case LEN_Z: x = (int64_t)       va_arg(va, size_t); break;
            case LEN_T: x =                 va_arg(va, ptrdiff_t); break;
            default: x =                    va_arg(va, int); break;
        }
        if (x < 0) {
            uval = (uint64_t)(-x);
            flags |= FL_NEG;
        } else uval = (uint64_t)x;
    } else {
        switch (len) {
            case LEN_HH: uval = (unsigned char)     va_arg(va, unsigned int); break;
            case LEN_H: uval = (unsigned short int) va_arg(va, unsigned int); break;
            case LEN_L: uval =                      va_arg(va, unsigned long int); break;
            case LEN_LL: uval =                     va_arg(va, unsigned long long int); break;
            case LEN_J: uval =                      va_arg(va, uintmax_t); break;
            case LEN_Z: uval =                      va_arg(va, size_t); break;
            case LEN_T: uval = (uint64_t)           va_arg(va, ptrdiff_t); break;
            default: uval =                         va_arg(va, unsigned int); break;
        }
    }

    const char *nstr = _ntoa(ctx, uval, base, flags & FL_UPPER);
    int nlen = (ctx->ntoa + NTOA_BUFFER_SIZE) - nstr;

    if (precision == 0 && uval == 0) nlen = 0;
    int prec_zeros = (precision > nlen) ? (precision - nlen) : 0;

    char sign = 0;
    if (flags & FL_NEG) sign = '-';
    else if ((flags & FL_ALWAYS_SIGN) && is_signed) sign = '+';
    else if (flags & FL_SIGN_PAD) sign = ' ';

    int prefix_type = 0; // 0-none  1-"0"  2-"0x"/"0X" // Also needs to amtch its length
    if (flags & FL_PREFIX) {
        if (base == 8 && prec_zeros == 0 && (uval != 0 || precision == 0)) {
            prefix_type = 1;
        } else if (base == 16 && uval != 0) {
            prefix_type = 2;
        }
    }

    int field = (sign ? 1 : 0) + prefix_type + prec_zeros + nlen;
    int pad_n = (width > field) ? (width - field) : 0;
    int zero_n = 0;
    if ((flags & FL_ZERO_PAD) && !(flags & FL_LEFT_JUSTIFY) && precision < 0) {
        zero_n = pad_n;
        pad_n  = 0;
    }

    if (!(flags & FL_LEFT_JUSTIFY)) _pad(ctx, ' ', pad_n);
    if (sign) _putc(ctx, sign);

    if (prefix_type) {
        _putc(ctx, '0');
        if (prefix_type == 2) _putc(ctx, (flags & FL_UPPER) ? 'X' : 'x');
    }

    _pad(ctx, '0', zero_n + prec_zeros);
    _str(ctx, nstr, nlen);
    if  (flags & FL_LEFT_JUSTIFY)   _pad(ctx, ' ', pad_n);
}

int _vprintkl(int level, const char *fmt, va_list va) {
    printk_context_t ctx = {
        .buf_len = 0,
        .level = level,
        .len = 0
    };

    while (*fmt) {
        if (*fmt != '%') { _putc(&ctx, *fmt++); continue; }
        const char* spec_start = fmt;
        fmt++;

        if (*fmt == '%') { _putc(&ctx, '%'); fmt++; continue; }

        unsigned flags = 0;
        for (;;) {
            switch (*fmt) {
                case '-': flags |= FL_LEFT_JUSTIFY; fmt++; continue;
                case '+': flags |= FL_ALWAYS_SIGN; fmt++; continue;
                case ' ': flags |= FL_SIGN_PAD; fmt++; continue;
                case '#': flags |= FL_PREFIX; fmt++; continue;
                case '0': flags |= FL_ZERO_PAD; fmt++; continue;
            }
            break;
        }

        int width = 0;
        if (*fmt == '*') {
            width = va_arg(va, int);
            if (width < 0) {
                width = -width;
                flags |= FL_LEFT_JUSTIFY;
            }
            fmt++;
        } else {
            while (*fmt >= '0' && *fmt <= '9')
                width = width * 10 + (*fmt++ - '0');
        }

        int precision = -1;
        if (*fmt == '.') {
            fmt++;
            if (*fmt == '*') {
                precision = va_arg(va, int);
                if (precision < 0) precision = -1;
                fmt++;
            } else {
                precision = 0;
                while (*fmt >= '0' && *fmt <= '9')
                    precision = precision * 10 + (*fmt++ - '0');
            }
        }

        length_t len = LEN_NONE;
        switch (*fmt) {
            case 'h': fmt++; len = (*fmt == 'h') ? (fmt++, LEN_HH) : LEN_H; break;
            case 'l': fmt++; len = (*fmt == 'l') ? (fmt++, LEN_LL) : LEN_L; break;
            case 'j': fmt++; len = LEN_J; break;
            case 'z': fmt++; len = LEN_Z; break;
            case 't': fmt++; len = LEN_T; break;
        }

        char spec = *fmt++;
        if (spec == '\0') break;

        switch (spec) {
            case 'd':
            case 'i': _integer(&ctx, 1, 10, flags, width, precision, len, va); break;
            case 'u': _integer(&ctx, 0, 10, flags, width, precision, len, va); break;
            case 'o': _integer(&ctx, 0, 8, flags, width, precision, len, va); break;
            case 'x': _integer(&ctx, 0, 16, flags, width, precision, len, va); break;
            case 'X': _integer(&ctx, 0, 16, flags | FL_UPPER, width, precision, len, va); break;

            case 'c': {
                wchar_t ch = (len == LEN_L) ? va_arg(va, wint_t) : va_arg(va, int);
                int pad = width - 1;
                if (!(flags & FL_LEFT_JUSTIFY) && pad > 0) _pad(&ctx, ' ', pad);
                if (len == LEN_L) _putwc(&ctx, ch); else _putc(&ctx, (char)ch);
                if ((flags & FL_LEFT_JUSTIFY) && pad > 0) _pad(&ctx, ' ', pad);
            } break;

            case 's': {
                const wchar_t* str;
                int str_len;
                if (len == LEN_L) {
                    str = va_arg(va, wchar_t*);
                    if (!str) str = L"(null)";
                    str_len = wcslen(str);
                } else {
                    str = (wchar_t*)va_arg(va, char*);
                    if (!str) str = (wchar_t*)"(null)";
                    str_len = strlen((const char*)str);
                }
                if (precision >= 0 && str_len > precision) str_len = precision;
                int pad = width - str_len;
                if (!(flags & FL_LEFT_JUSTIFY) && pad > 0) _pad(&ctx, ' ', pad);
                if (len == LEN_L) _wstr(&ctx, str, str_len); else _str(&ctx, (const char*)str, str_len);
                if ((flags & FL_LEFT_JUSTIFY) && pad > 0)  _pad(&ctx, ' ', pad);
            } break;

            case 'p': {
                uintptr_t ptr = (uintptr_t)va_arg(va, void*);
                const char *nstr = _ntoa(&ctx, (uint64_t)ptr, 16, 0);
                int nlen = (ctx.ntoa + NTOA_BUFFER_SIZE) - nstr;
                int ptr_digs = (int)(sizeof(void*) * 2);
                int zero_n = (nlen < ptr_digs) ? ptr_digs - nlen : 0;
                int field = 2 + zero_n + nlen;
                int pad = width - field;
                if (!(flags & FL_LEFT_JUSTIFY) && pad > 0) _pad(&ctx, ' ', pad);
                _str(&ctx, "0x", 2);
                _pad(&ctx, '0', zero_n);
                _str(&ctx, nstr, nlen);
                if ((flags & FL_LEFT_JUSTIFY) && pad > 0)  _pad(&ctx, ' ', pad);
            } break;

            case 'n': {
                void *ptr = va_arg(va, void*);
                if (!ptr) break;
                switch (len) {
                    case LEN_HH: *(signed char*)ptr = (signed char)ctx.len; break;
                    case LEN_H: *(short int*)ptr = (short int)ctx.len; break;
                    case LEN_L: *(long int*)ptr = (long int)ctx.len; break;
                    case LEN_LL: *(long long int*)ptr = (long long int)ctx.len; break;
                    case LEN_J: *(intmax_t*)ptr = (intmax_t)ctx.len; break;
                    case LEN_Z: *(size_t*)ptr = (size_t)ctx.len; break;
                    case LEN_T: *(ptrdiff_t*)ptr = (ptrdiff_t)ctx.len; break;
                    default: *(int*)ptr = ctx.len; break;
                }
            } break;

            default:
                _str(&ctx, spec_start, (fmt - spec_start));
        }
    }

    _flush(&ctx);
    return ctx.len;
}

int vprintkl(int level, const char *fmt, va_list args) {
    if (!fmt) return 0;
    return _vprintkl(level, fmt, args);
}

int vprintk(const char *fmt, va_list args) {
    if (!fmt) return 0;
    int level = DEFAULT_LEVEL;
    if ((unsigned char)fmt[0] == 0x01 && fmt[1] >= '0' && fmt[1] <= '9') {
        level  = fmt[1] - '0';
        fmt   += 2;
    }
    return vprintkl(level, fmt, args);
}

int printkl(int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vprintkl(level, fmt, args);
    va_end(args);
    return ret;
}

int printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vprintk(fmt, args);
    va_end(args);
    return ret;
}

printk_sink_t *printk_sink_register(printk_sink_t sink) {
    if (!sink.name[0]) return NULL;
    for (int i = 0; i < MAX_SINKS; i++) {
        if (!sinks[i].name[0]) {
            sinks[i] = sink;
            return sinks + i;
        }
    }
    return NULL;
}

int printk_sink_unregister(printk_sink_t *sink) {
    if (!sink || sink < sinks || sink >= sinks + MAX_SINKS) return -1;
    sink->name[0] = '\0';
    return 0;
}