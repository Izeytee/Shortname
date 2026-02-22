#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "LTEmod.h"

int main()
{
//#pragma omp parallel
{
	const size_t N = 80000000;

	uint32_t *src = malloc(N * 4);

	for (int i = 0; i < N; ++i)
	{
		src[i] = i % 256;
	}

	float *real = malloc(N * 4); 
	float *img = malloc(N * 4);

	T32fc *dst = malloc(N * 8);

	modulation(src, dst, N);
	//modulation_full_opt(src, real, img, N);
	//modulation_opt(src, dst, N);


}
}
