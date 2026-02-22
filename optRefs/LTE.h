#include <stdint.h>

typedef struct
{
	float re;
	float im;
} T32fc;

void modulation(const uint32_t *pSrc, T32fc *pDst, uint32_t length);
void modulation_opt(const uint32_t *pSrc, T32fc *pDst, uint32_t length);
void modulation_full_opt(const uint32_t *pSrc, float *pDstReal, float *pDstImg, uint32_t length);
void modulation_absolute_opt(const uint32_t *pSrc, T32fc *pDst, uint32_t length);
void modulation_not_opt(const uint32_t *pSrc, float *pDstReal, float *pDstImg, uint32_t length);

void calcFreqOffset(T32fc* pSrcRx[4], T32fc *pSrcTx[2], uint32_t len, float *pCfo, float tLimit, float subCrSpace);
void calcFreqOffset_opt(float* rxre[4], float *rxIm[2], float *txRe[2], float *txIm[2], uint32_t len, float *pCfo, float tLimit, float subCrSpace);

