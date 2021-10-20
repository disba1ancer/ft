/*
 * main.cpp
 *
 *  Created on: 13 сент. 2020 г.
 *      Author: disba1ancer
 */

#include "ft.h"
#include <cmath>
#include "extmath.h"

namespace ft {

FFT::FFT(unsigned logSize) : logSize(logSize), size(1 << logSize), exps(size / 2), data(size) {
	exps[0] = std::complex<float>(1, 0);
	exps[1] = std::exp(std::complex<float>(0, -fract(1 / float(size)) * 2.f * Pi));
	for (unsigned i = 2; i < size / 2; ++i) {
		exps[i] = exps[1] * exps[i - 1];
	}
}

void FFT::operator()(const float* in, float* out, std::size_t start, std::size_t mask) {
	if (!mask) mask = size - 1;
	auto shift = sizeof(std::size_t) * CHAR_BIT - logSize;
	auto size = this->size;
	for (std::size_t i = 0, elemNum0 = 0, elemNum1 = invInc(elemNum0); i < size; i += 2, elemNum0 = invInc(elemNum1), elemNum1 = invInc(elemNum0)) {
		auto a = in[(start + (elemNum0 >> shift)) & mask];
		auto b = in[(start + (elemNum1 >> shift)) & mask];
		data[i] = a + b;
		data[i + 1] = a - b;
	}
	for (std::size_t cols = size / 2, rows = 2, expind = cols / 2; cols != 2; cols = expind, expind /= 2, rows *= 2) {
		for (std::size_t col = 0; col < cols; col += 2) {
			for (std::size_t row = 0; row < rows; ++row) {
				auto a = data[calcPos(row, col, rows, cols)];
				auto b = data[calcPos(row, col + 1, rows, cols)];
				auto w = exps[row * expind];
				data[calcPos(row, col, rows, cols)] = a + b * w;
				data[calcPos(row, col + 1, rows, cols)] = a - b * w;
			}
		}
	}
	for (std::size_t i = 0; i < size / 2; ++i) {
		auto a = data[i] + data[size / 2 + i] * exps[i];
		out[i] = std::abs(a) / size;
		out[i + size / 2] = std::arg(a);
	}
}

std::size_t FFT::invInc(std::size_t v) {
	auto t = (std::size_t(1) << (CHAR_BIT * sizeof(std::size_t) - 1));
	v ^= t;
	if (!(v & t)) {
		do {
			t >>= 1;
			v ^= t;
		} while (v < t);
	}
	return v;
}

std::size_t FFT::calcPos(std::size_t x, std::size_t y, std::size_t sx, std::size_t sy) {
	return y * sx + x;
}

void ft(const float* in, float* out, std::size_t size) {
	std::complex<float> shift0 = std::exp(std::complex<float>(0, - 2.f * Pi / float(size)));
	std::complex<float> shift(1, 0);
	for (int freq = 0; freq < size / 2; ++freq) {
		//float a = 0, ia = 0;
		std::complex<float> result(0, 0);
		std::complex<float> start(1, 0);
		//std::complex<float> shift = std::exp(std::complex<float>(0, -fract(freq * freqmult / float(size)) * 2.f * Pi));
		for (int i = 0; i < size; ++i) {
			//a += std::cos(fract(i * freq * freqmult / float(size)) * 2.f * Pi) * in[i] / size;
			//ia -= std::sin(fract(i * freq * freqmult / float(size)) * 2.f * Pi) * in[i] / size;
			result += /*std::exp(std::complex<float>(0, -fract(i * freq * freqmult / float(size)) * 2.f * Pi))*/start * in[i];
			start *= shift;
		}
		result /= float(size);
		out[size / 2 + freq] = 0;
		//float t = (a * a + ia * ia);
		//if (t > .000001f) {
			//out[freq] = std::sqrt(t);
			out[freq] = std::abs(result);
			out[size / 2 + freq] = std::arg(result);
		/*} else {
			out[freq] = 0;
		}*/
		shift *= shift0;
	}
}

} // namespace ft
