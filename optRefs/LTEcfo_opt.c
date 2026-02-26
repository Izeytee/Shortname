#include <stdint.h>
#include <math.h>

// LTE Calculate Frequency Offset estimation

typedef struct {
    float re;
    float im;
} T32fc;

/* //##########################################################################
For integer case: replace T32fc with T16sc:
typedef struct
{
    int16_t re;
    int16_t im;
} T16sc;

Result is expected in Q15 format (int16_t in format [1-bit sign][decimal dot][15-bit mantissa]).
"len" is multiple of 12, max len = 1200
*/ //##########################################################################

static __inline T32fc __attribute__((__gnu_inline__, __always_inline__))
mulConj_32fc(T32fc a, T32fc b)
{
    T32fc c;
    c.re = a.re * b.re + a.im * b.im;
    c.im = a.im * b.re - a.re * b.im;
    return c;
}

static __inline T32fc __attribute__((__gnu_inline__, __always_inline__))
maddCplx_32fc(T32fc a, T32fc b, T32fc c)
{
    c.re += a.re * b.re - a.im * b.im;
    c.im += a.im * b.re + a.re * b.im;
    return c;
}

#define OWN_PI    ( 3.14159265358979323846 )  // PI

static const float timeLimitsSlotInSec = 0.0005f;               // tLimit
static const float cellConfigRrcSubCarrierSpacingHz = 15000.f;  // subCrSpace

void calcFreqOffset(T32fc *pSrcRx[4], T32fc *pSrcTx[2], uint32_t len, float *pCfo, float tLimit, float subCrSpace)
{
    T32fc tmp[2];
    T32fc sum = {0, 0};

    for (uint32_t i = 0; i < len; i++)
    {
        tmp[0] = mulConj_32fc(pSrcTx[0][i], pSrcRx[0][i]);
        tmp[1] = mulConj_32fc(pSrcRx[2][i], pSrcTx[1][i]);

        sum = maddCplx_32fc(tmp[0], tmp[1], sum);

        tmp[0] = mulConj_32fc(pSrcTx[0][i], pSrcRx[1][i]);
        tmp[1] = mulConj_32fc(pSrcRx[3][i], pSrcTx[1][i]);

        sum = maddCplx_32fc(tmp[0], tmp[1], sum);
    }
    *pCfo = atan2(sum.im, sum.re);
    *pCfo /= 2 * OWN_PI * tLimit * subCrSpace;
}

#include <riscv_vector.h>

void calcFreqOffset_opt(T32fc *pSrcRx[4], T32fc *pSrcTx[2], uint32_t len, float *pCfo, float tLimit, float subCrSpace)
{
    size_t vl = __riscv_vsetvl_e32m1(len);
    
    vfloat32m1_t vsum_re = __riscv_vfmv_v_f_f32m1(0.0f, vl);
    vfloat32m1_t vsum_im = __riscv_vfmv_v_f_f32m1(0.0f, vl);
    
    float divisor = 2.0f * OWN_PI * tLimit * subCrSpace;
    
    const float *rx0 = (float*)pSrcRx[0];
    const float *rx1 = (float*)pSrcRx[1];
    const float *rx2 = (float*)pSrcRx[2];
    const float *rx3 = (float*)pSrcRx[3];
    const float *tx0 = (float*)pSrcTx[0];
    const float *tx1 = (float*)pSrcTx[1];
    
    for (uint32_t i = 0; i + vl <= len; i += vl)
    {
        vl = __riscv_vsetvl_e32m1(len - i);
        
        vfloat32m1x2_t v_tmp0 = __riscv_vlseg2e32_v_f32m1x2(tx0 + i * 2, vl);
        vfloat32m1_t v_tx0_re = __riscv_vget_v_f32m1x2_f32m1(v_tmp0, 0);
        vfloat32m1_t v_tx0_im = __riscv_vget_v_f32m1x2_f32m1(v_tmp0, 1);

        vfloat32m1x2_t v_tmp1 = __riscv_vlseg2e32_v_f32m1x2(tx1 + i * 2, vl);
        vfloat32m1_t v_tx1_re = __riscv_vget_v_f32m1x2_f32m1(v_tmp1, 0);
        vfloat32m1_t v_tx1_im = __riscv_vget_v_f32m1x2_f32m1(v_tmp1, 1);
        
        vfloat32m1x2_t v_tmp2 = __riscv_vlseg2e32_v_f32m1x2(rx0 + i * 2, vl);
        vfloat32m1_t v_rx0_re = __riscv_vget_v_f32m1x2_f32m1(v_tmp2, 0);
        vfloat32m1_t v_rx0_im = __riscv_vget_v_f32m1x2_f32m1(v_tmp2, 1);

        vfloat32m1x2_t v_tmp3 = __riscv_vlseg2e32_v_f32m1x2(rx1 + i * 2, vl);
        vfloat32m1_t v_rx1_re = __riscv_vget_v_f32m1x2_f32m1(v_tmp3, 0);
        vfloat32m1_t v_rx1_im = __riscv_vget_v_f32m1x2_f32m1(v_tmp3, 1);

        vfloat32m1x2_t v_tmp4 = __riscv_vlseg2e32_v_f32m1x2(rx2 + i * 2, vl);
        vfloat32m1_t v_rx2_re = __riscv_vget_v_f32m1x2_f32m1(v_tmp4, 0);
        vfloat32m1_t v_rx2_im = __riscv_vget_v_f32m1x2_f32m1(v_tmp4, 1);

        vfloat32m1x2_t v_tmp5 = __riscv_vlseg2e32_v_f32m1x2(rx3 + i * 2, vl);
        vfloat32m1_t v_rx3_re = __riscv_vget_v_f32m1x2_f32m1(v_tmp5, 0);
        vfloat32m1_t v_rx3_im = __riscv_vget_v_f32m1x2_f32m1(v_tmp5, 1);
        
        vfloat32m1_t v_t0_re = __riscv_vfmacc_vv_f32m1(__riscv_vfmul_vv_f32m1(v_tx0_re, v_rx0_re, vl), 
                                                v_tx0_im, v_rx0_im, vl);
        vfloat32m1_t v_t0_im = __riscv_vfmsac_vv_f32m1(__riscv_vfmul_vv_f32m1(v_tx0_im, v_rx0_re, vl), 
                                                v_tx0_re, v_rx0_im, vl);
        
        vfloat32m1_t v_t1_re = __riscv_vfmacc_vv_f32m1(__riscv_vfmul_vv_f32m1(v_rx2_re, v_tx1_re, vl), 
                                                v_rx2_im, v_tx1_im, vl);
        vfloat32m1_t v_t1_im = __riscv_vfmsac_vv_f32m1(__riscv_vfmul_vv_f32m1(v_rx2_im, v_tx1_re, vl), 
                                                v_rx2_re, v_tx1_im, vl);
        
        vsum_re = __riscv_vfmacc_vv_f32m1(__riscv_vfmsac_vv_f32m1(vsum_re, v_t0_im, v_t1_im, vl), 
                                   v_t0_re, v_t1_re, vl);
        vsum_im = __riscv_vfmacc_vv_f32m1(__riscv_vfmacc_vv_f32m1(vsum_im, v_t0_re, v_t1_im, vl), 
                                   v_t0_im, v_t1_re, vl);
        
        v_t0_re = __riscv_vfmacc_vv_f32m1(__riscv_vfmul_vv_f32m1(v_tx0_re, v_rx1_re, vl), 
                                   v_tx0_im, v_rx1_im, vl);
        v_t0_im = __riscv_vfmsac_vv_f32m1(__riscv_vfmul_vv_f32m1(v_tx0_im, v_rx1_re, vl), 
                                   v_tx0_re, v_rx1_im, vl);
        
        v_t1_re = __riscv_vfmacc_vv_f32m1(__riscv_vfmul_vv_f32m1(v_rx3_re, v_tx1_re, vl), 
                                   v_rx3_im, v_tx1_im, vl);
        v_t1_im = __riscv_vfmsac_vv_f32m1(__riscv_vfmul_vv_f32m1(v_rx3_im, v_tx1_re, vl), 
                                   v_rx3_re, v_tx1_im, vl);
        
        vsum_re = __riscv_vfmacc_vv_f32m1(__riscv_vfmsac_vv_f32m1(vsum_re, v_t0_im, v_t1_im, vl), 
                                   v_t0_re, v_t1_re, vl);
        vsum_im = __riscv_vfmacc_vv_f32m1(__riscv_vfmacc_vv_f32m1(vsum_im, v_t0_re, v_t1_im, vl),
                                   v_t0_im, v_t1_re, vl);
    }
    
    float sum_re = __riscv_vfmv_f_s_f32m1_f32(__riscv_vfredosum_vs_f32m1_f32m1(vsum_re, __riscv_vfmv_v_f_f32m1(0.0f, vl), vl));
    float sum_im = __riscv_vfmv_f_s_f32m1_f32(__riscv_vfredosum_vs_f32m1_f32m1(vsum_im, __riscv_vfmv_v_f_f32m1(0.0f, vl), vl));
    
    // Финальное вычисление
    *pCfo = atan2f(sum_im, sum_re);
    *pCfo /= divisor;
}

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
int main()
{
    const int N = 1280000;

    T32fc *pSrcRx[4];
    for (int i = 0; i < 4; i++)
    {
        pSrcRx[i] = (T32fc*)malloc(N * sizeof(T32fc));
        for (int j = 0; j < N; j++)
        {
            pSrcRx[i][j].re = rand() / (float)RAND_MAX;
            pSrcRx[i][j].im = rand() / (float)RAND_MAX;
        }
    }

    T32fc *pSrcTx[2];
    for (int i = 0; i < 2; i++)
    {
        pSrcTx[i] = (T32fc*)malloc(N * sizeof(T32fc));
        for (int j = 0; j < N; j++)
        {
            pSrcTx[i][j].re = rand() / (float)RAND_MAX;
            pSrcTx[i][j].im = rand() / (float)RAND_MAX;
        }
    }

    float pCfo;
    float tLimit = 0.01f;
    float subCrSpace = 0.01f;

    
    double t;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    calcFreqOffset(pSrcRx, pSrcTx, N, &pCfo, tLimit, subCrSpace);
    clock_gettime(CLOCK_MONOTONIC, &end);
    t = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    printf("Time taken: %e\n", t);
}