#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "LTEmod.h"

int main()
{
//#pragma omp parallel
{
	const size_t N = 64000000;

	uint32_t *src = malloc(N * 4);

	for (int i = 0; i < N; ++i)
	{
		src[i] = i % 16;
//		printf("%i ", src[i]);
	}

//	printf("\n");

	float *real = malloc(N * 4); 
	float *img = malloc(N * 4);

	T32fc *dst = malloc(N * 8);

	//modulation(src, dst, N);
	//modulation_full_opt(src, real, img, N);
	modulation_absolute_opt(src, dst, N);
	//modulation_not_opt(src, real, img, N);
	
	//modulation_opt(src, dst, N);


	for (int i = 0; i < N; ++i)
	{
		//printf("%f %f\n", dst[i].re, dst[i].im);
	}
}
}
