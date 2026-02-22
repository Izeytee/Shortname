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

void calcFreqOffset_opt(const float * rxRe[4], 
                              const float * rxIm[4],
                              const float * txRe[2], 
                              const float * txIm[2],
                              uint32_t len, 
                              float * pCfo, 
                              float tLimit, 
                              float subCrSpace)
{
    // Настройка векторного режима (LMUL=1, SEW=32-bit float)
    size_t vl = __riscv_vsetvl_e32m1(len);
    
    // Векторные аккумуляторы для суммы (инициализация нулями)
    vfloat32m1_t vsum_re = __riscv_vfmv_v_f_f32m1(0.0f, vl);
    vfloat32m1_t vsum_im = __riscv_vfmv_v_f_f32m1(0.0f, vl);
    
    // Предвычисляем знаменатель
    float divisor = 2.0f * OWN_PI * tLimit * subCrSpace;
    
    // Кэшируем указатели
    const float *rx0_re = rxRe[0];
    const float *rx0_im = rxIm[0];
    const float *rx1_re = rxRe[1];
    const float *rx1_im = rxIm[1];
    const float *rx2_re = rxRe[2];
    const float *rx2_im = rxIm[2];
    const float *rx3_re = rxRe[3];
    const float *rx3_im = rxIm[3];
    const float *tx0_re = txRe[0];
    const float *tx0_im = txIm[0];
    const float *tx1_re = txRe[1];
    const float *tx1_im = txIm[1];
    
    uint32_t i = 0;
    
    // Основной векторный цикл
    for (; i + vl <= len; i += vl)
    {
        // Обновляем векторную длину для остатка
        vl = __riscv_vsetvl_e32m1(len - i);
        
        // --- Загрузка данных в векторные регистры ---
        vfloat32m1_t v_tx0_re = __riscv_vle32_v_f32m1(tx0_re + i, vl);
        vfloat32m1_t v_tx0_im = __riscv_vle32_v_f32m1(tx0_im + i, vl);
        vfloat32m1_t v_tx1_re = __riscv_vle32_v_f32m1(tx1_re + i, vl);
        vfloat32m1_t v_tx1_im = __riscv_vle32_v_f32m1(tx1_im + i, vl);
        
        vfloat32m1_t v_rx0_re = __riscv_vle32_v_f32m1(rx0_re + i, vl);
        vfloat32m1_t v_rx0_im = __riscv_vle32_v_f32m1(rx0_im + i, vl);
        vfloat32m1_t v_rx1_re = __riscv_vle32_v_f32m1(rx1_re + i, vl);
        vfloat32m1_t v_rx1_im = __riscv_vle32_v_f32m1(rx1_im + i, vl);
        vfloat32m1_t v_rx2_re = __riscv_vle32_v_f32m1(rx2_re + i, vl);
        vfloat32m1_t v_rx2_im = __riscv_vle32_v_f32m1(rx2_im + i, vl);
        vfloat32m1_t v_rx3_re = __riscv_vle32_v_f32m1(rx3_re + i, vl);
        vfloat32m1_t v_rx3_im = __riscv_vle32_v_f32m1(rx3_im + i, vl);
        
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
    
    // --- Редукция: суммируем все элементы вектора в скаляр ---
    float sum_re = __riscv_vfmv_f_s_f32m1_f32(__riscv_vfredosum_vs_f32m1_f32m1(vsum_re, __riscv_vfmv_v_f_f32m1(0.0f, vl), vl));
    float sum_im = __riscv_vfmv_f_s_f32m1_f32(__riscv_vfredosum_vs_f32m1_f32m1(vsum_im, __riscv_vfmv_v_f_f32m1(0.0f, vl), vl));
    
    // Финальное вычисление
    *pCfo = atan2f(sum_im, sum_re);
    *pCfo /= divisor;
}