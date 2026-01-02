#include "ctype.h"

bool is_lower(char chr) {
    return chr >= 'a' && chr <= 'z';
}

char to_upper(char chr) {
    return is_lower(chr) ? (chr - 'a' + 'A') : chr;
}
