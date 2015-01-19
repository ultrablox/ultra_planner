
#ifndef UltraCore_math_h
#define UltraCore_math_h

#include <math.h>
#include <cstddef>

//Return x: x^10 = val!
template<typename IntT>
size_t lg_factor(IntT val)
{
	double res = 0.0;
	for (IntT i = 2; i <= val; ++i)
		res += log10((double)i);

	
	return size_t(ceil(res));
}

#endif
