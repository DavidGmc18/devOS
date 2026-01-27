#include <stdio.h>
#include <arch/i686/io.h>
#include <stdarg.h>
#include <stdbool.h>

void fputc(char ch, fd_t stream) {
    VFS_Write(stream, &ch, sizeof(ch));
}

void fputs(const char* str, fd_t stream) {
    while (*str) {
        fputc(*str, stream);
        str++;
    }
}

void fputn(const char* str, fd_t stream, size_t size) {
    VFS_Write(stream, str, sizeof(char) * size);
}

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

const char g_HexChars[] = "0123456789abcdef";

#define PRINTF_UNSIGNED_BUFFER_SIZE 32
void fprintf_unsigned(fd_t stream, unsigned long long number, int radix) {
    char buffer[PRINTF_UNSIGNED_BUFFER_SIZE];
    int pos = 0;

    // convert number to ASCII
    do {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[PRINTF_UNSIGNED_BUFFER_SIZE-1-pos++] = g_HexChars[rem];
    } while (number > 0);

    fputn(buffer + PRINTF_UNSIGNED_BUFFER_SIZE - pos, stream, pos);
}

void fprintf_signed(fd_t stream, long long number, int radix) {
    if (number < 0) {
        fputc('-', stream);
        fprintf_unsigned(stream, -number, radix);
    } else {
        fprintf_unsigned(stream, number, radix);
    }
}

void vfprintf(fd_t stream, const char* format, va_list args) {
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;
    const char* seq_start = NULL;

    while (*format) {
        switch (state) {
            case PRINTF_STATE_NORMAL:
                switch (*format) {
                    case '%':   state = PRINTF_STATE_LENGTH;
                                if (seq_start != NULL) {
                                    fputn(seq_start, stream, format - seq_start);
                                    seq_start = NULL;
                                }
                                break;

                    default:    if (seq_start == NULL) {
                                    seq_start = format;
                                }
                                break;
                }
                break;

            case PRINTF_STATE_LENGTH:
                switch (*format) {
                    case 'h':   length = PRINTF_LENGTH_SHORT;
                                state = PRINTF_STATE_LENGTH_SHORT;
                                break;
                    case 'l':   length = PRINTF_LENGTH_LONG;
                                state = PRINTF_STATE_LENGTH_LONG;
                                break;
                    default:    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT:
                if (*format == 'h') {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                } else {
                    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if (*format == 'l') {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                } else {
                    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch (*format) {
                    case 'c':   fputc((char)va_arg(args, int), stream);
                                break;

                    case 's':   
                                fputs(va_arg(args, const char*), stream);
                                break;

                    case '%':   fputc('%', stream);
                                break;

                    case 'd':
                    case 'i':   radix = 10; sign = true; number = true;
                                break;

                    case 'u':   radix = 10; sign = false; number = true;
                                break;

                    case 'X':
                    case 'x':
                    case 'p':   radix = 16; sign = false; number = true;
                                break;

                    case 'o':   radix = 8; sign = false; number = true;
                                break;

                    // ignore invalid spec
                    default:    break;
                }

                if (number) {
                    if (sign) {
                        switch (length) {
                            case PRINTF_LENGTH_SHORT_SHORT:
                            case PRINTF_LENGTH_SHORT:
                            case PRINTF_LENGTH_DEFAULT:     fprintf_signed(stream, va_arg(args, int), radix);
                                                            break;

                            case PRINTF_LENGTH_LONG:        fprintf_signed(stream, va_arg(args, long), radix);
                                                            break;

                            case PRINTF_LENGTH_LONG_LONG:   fprintf_signed(stream, va_arg(args, long long), radix);
                                                            break;
                        }
                    } else {
                        switch (length) {
                            case PRINTF_LENGTH_SHORT_SHORT:
                            case PRINTF_LENGTH_SHORT:
                            case PRINTF_LENGTH_DEFAULT:     fprintf_unsigned(stream, va_arg(args, unsigned int), radix);
                                                            break;
                                                            
                            case PRINTF_LENGTH_LONG:        fprintf_unsigned(stream, va_arg(args, unsigned  long), radix);
                                                            break;

                            case PRINTF_LENGTH_LONG_LONG:   fprintf_unsigned(stream, va_arg(args, unsigned  long long), radix);
                                                            break;
                        }
                    }
                }

                // reset state
                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
        }

        format++;
    }

    if (seq_start != NULL) {
        fputn(seq_start, stream, format - seq_start);
    }
}

void fprintf(fd_t stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);
}

void putc(char ch) {
    fputc(ch, VFS_FD_STDOUT);
}

void puts(const char* str) {
    fputs(str, VFS_FD_STDOUT);
}

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(VFS_FD_STDOUT, format, args);
    va_end(args);
}