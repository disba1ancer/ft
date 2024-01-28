/*
 * ft.h
 *
 *  Created on: 13 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef FT_H_
#define FT_H_

#include <cstddef>
#include <vector>
#include <complex>

namespace ft {

/*
 * Fast Fourier transform algorithm
 */
class FFT {
public:
	FFT(std::size_t maxSize);
	void operator()(const float* in, float* out, std::size_t start = 0, std::size_t offsetMask = 0, bool absModDecomp = false);
	void transform(const float* in, float* out, std::size_t start = 0, std::size_t offsetMask = 0, bool unpack = true);
	void reverse(const float* in, float* out, std::size_t start = 0, std::size_t offsetMask = 0);
private:
	std::size_t invInc(std::size_t v);
	static std::size_t invInc(std::size_t v, std::size_t size);
	static std::size_t calcPos(std::size_t x, std::size_t y, std::size_t sx, std::size_t sy);
private:
	std::size_t maxSize;
	std::vector<std::complex<float>> exps;
	std::vector<std::complex<float>> data;
};

/*
 * Fourier transform simple algorithm ( O(n²) )
 */
void ft(const float* in, float* out, std::size_t size);

} // namespace ft

#endif /* FT_H_ */
