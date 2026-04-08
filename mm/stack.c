#include "stack.h"

__attribute__((aligned(16))) unsigned char kern_stack[4096];
__attribute__((aligned(16))) unsigned char emergency_stack[4096];