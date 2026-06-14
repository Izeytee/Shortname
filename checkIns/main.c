#include <stdio.h>
#include <stdint.h>

static uint64_t getCycles()
{
   uint64_t cycles = 0;
   asm volatile("csrr %0, 0xc01" : "=r"(cycles));
   return cycles;
}

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

#define CHECK_OPER(INSTR, TYPE, OPER, RET)                  \
do {                                                        \
    uint64_t start = getCycles();                           \
    asm volatile(                                           \
        "vsetvli t0, zero, e16, m1 \n\t"                    \
        UNROLL_4096(OPER)                                   \
    );                                                      \
    uint64_t end = getCycles();                             \
    RET = end - start;                                      \
} while (0)

int main()
{
    const size_t N = 256;
    double average[1024];
    for (int i = 0; i < 1024; ++i)
    {
	average[i] = 0.0;
    }
    uint64_t elapsed;
    for (int i = 0; i < N; ++i)
    {
    	CHECK_OPER("vand.vi", "LATENCY", LATENCY_SCALAR_BLOCK("vand.vi", "v"), elapsed);
	average[0] += elapsed;
        CHECK_OPER("vand.vi", "THROUGHPUT", THROUGHPUT_SCALAR_BLOCK("vand.vi", "v"), elapsed);    
	average[1] += elapsed;

    	CHECK_OPER("vsrl.vi", "LATENCY", LATENCY_SCALAR_BLOCK("vsrl.vi", "v"), elapsed);
	average[2] += elapsed;
        CHECK_OPER("vsrl.vi", "THROUGHPUT", THROUGHPUT_SCALAR_BLOCK("vsrl.vi", "v"), elapsed);    
	average[3] += elapsed;

    	CHECK_OPER("vor.vv", "LATENCY", LATENCY_BLOCK("vor.vv", "v"), elapsed);
	average[4] += elapsed;
        CHECK_OPER("vor.vv", "THROUGHPUT", THROUGHPUT_BLOCK("vor.vv", "v"), elapsed);    
	average[5] += elapsed;

	//CHECK_OPER(LATENCY_SCALAR_BLOCK("vsrl,vi", "v"), elapsed);
    	//CHECK_OPER(THROUGHPUT_SCALAR_BLOCK, elapsed);

    	//CHECK_OPER("vor.vv", "v", LATENCY_BLOCK, elapsed);
    	//CHECK_OPER("vor.vv", "v", THROUGHPUT_BLOCK, elapsed);
    }

    for (size_t i = 0; i < 6; ++i)
    {
    	printf("average: %f\n", average[i] / N);
    }
    //CHECK_LATENCY("vrgather.vv", "v", elapsed);
    //CHECK_THROUGHPUT("vrgather.vv", "v", elapsed);
}
