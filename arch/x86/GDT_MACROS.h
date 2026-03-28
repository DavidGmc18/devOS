#pragma once

#define A(v) (v&1)
#define RW(v) ((v&1) << 1)
#define DC(v) ((v&1) << 2)
#define E(v) ((v&1) << 3)
#define S(v) ((v&1) << 4)
#define DPL(v) ((v&3) << 5)
#define P(v) ((v&1) << 7)

#define LIMIT_HIGH(v) (v&0xF)
#define L(v) ((v&1) << 5)
#define DB(v) ((v&1) << 6)
#define G(v) ((v&1) << 7)