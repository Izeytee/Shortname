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
