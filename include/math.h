#pragma once

#define ROUND_DOWN(a, b) ({         \
    __typeof__(a) _a = (a);         \
    __typeof__(b) _b = (b);         \
    ((_a / _b) * _b);               \
})

#define ROUND_UP(a, b) ({           \
    __typeof__(a) _a = (a);         \
    __typeof__(b) _b = (b);         \
    (((_a + _b - 1) / _b) * _b);    \
})

#define DIV_UP(a, b) ({             \
    __typeof__(a) _a = (a);         \
    __typeof__(b) _b = (b);         \
    ((_a + _b - 1) / _b);           \
})
