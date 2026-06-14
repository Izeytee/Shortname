#include <stdio.h>

#define UNROLL_4096(X)  \
    UNROLL_1024(X)      \
    UNROLL_1024(X)      \
    UNROLL_1024(X)      \
    UNROLL_1024(X)

#define UNROLL_1024(X)  \
    UNROLL_256(X)       \
    UNROLL_256(X)       \
    UNROLL_256(X)       \
    UNROLL_256(X)

#define UNROLL_256(X)   \
    UNROLL_64(X)        \
    UNROLL_64(X)        \
    UNROLL_64(X)        \
    UNROLL_64(X)

#define UNROLL_64(X)   \
    UNROLL_16(X)       \
    UNROLL_16(X)       \
    UNROLL_16(X)       \
    UNROLL_16(X)

#define UNROLL_16(X)    \
    UNROLL_4(X)         \
    UNROLL_4(X)         \
    UNROLL_4(X)         \
    UNROLL_4(X)

#define UNROLL_4(X) \
    X               \
    X               \
    X               \
    X

#define LATENCY_BLOCK(X, REG_TYPE) \
    X" "REG_TYPE"0, "REG_TYPE"1, "REG_TYPE"2 \n\t"

#define THROUGHPUT_BLOCK(X, REG_TYPE) \
    X" "REG_TYPE"0, "REG_TYPE"1, "REG_TYPE"2 \n\t"          \
    X" "REG_TYPE"3, "REG_TYPE"4, "REG_TYPE"5 \n\t"          \
    X" "REG_TYPE"6, "REG_TYPE"7, "REG_TYPE"8 \n\t"          \
    X" "REG_TYPE"9, "REG_TYPE"10, "REG_TYPE"11 \n\t"        \
    X" "REG_TYPE"12, "REG_TYPE"13, "REG_TYPE"14 \n\t"       \
    X" "REG_TYPE"15, "REG_TYPE"16, "REG_TYPE"17 \n\t"       \
    X" "REG_TYPE"18, "REG_TYPE"19, "REG_TYPE"20 \n\t"       \

#define LATENCY_SCALAR_BLOCK(X, REG_TYPE) \
    X" "REG_TYPE"0, "REG_TYPE"1, 3 \n\t"

#define THROUGHPUT_SCALAR_BLOCK(X, REG_TYPE) \
    X" "REG_TYPE"0, "REG_TYPE"1, 3 \n\t"         \
    X" "REG_TYPE"3, "REG_TYPE"4, 3 \n\t"         \
    X" "REG_TYPE"6, "REG_TYPE"7, 3 \n\t"         \
    X" "REG_TYPE"9, "REG_TYPE"10, 3 \n\t"        \
    X" "REG_TYPE"12, "REG_TYPE"13, 3 \n\t"       \
    X" "REG_TYPE"15, "REG_TYPE"16, 3 \n\t"       \
    X" "REG_TYPE"18, "REG_TYPE"19, 3 \n\t"       \

#define CHECK_OPER(INSTR, REG_TYPE, OPER, RET)              \
do {                                                        \
    long start = getTime();                                 \
    asm volatile(                                           \
        "vsetvli t0, zero, e16, m1 \n\t"                    \
        UNROLL_16(OPER(INSTR, REG_TYPE))                    \
    );                                                      \
    long end = getTime();                                   \
    RET = start - end;                                      \
    printf(OPER" for "INSTR": %ld cycles\n", RET);          \
} while (0)

int main()
{
    long elapsed;
    CHECK_OPER("vand.vi", "v", LATENCY_SCALAR_BLOCK, elapsed);
    CHECK_OPER("vand.vi", "v", THROUGHPUT_SCALAR_BLOCK, elapsed);

    CHECK_OPER("vsrl.vi", "v", LATENCY_SCALAR_BLOCK, elapsed);
    CHECK_OPER("vsrl.vi", "v", THROUGHPUT_SCALAR_BLOCK, elapsed);

    CHECK_OPER("vor.vv", "v", LATENCY_BLOCK, elapsed);
    CHECK_OPER("vor.vv", "v", THROUGHPUT_BLOCK, elapsed);

    //CHECK_LATENCY("vrgather.vv", "v", elapsed);
    //CHECK_THROUGHPUT("vrgather.vv", "v", elapsed);
}