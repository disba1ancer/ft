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

FFT::FFT(std::size_t size) : maxSize(size), exps(size / 2), data(size) {
	exps[0] = {1, 0};
	exps[1] = std::exp(std::complex<float>(0, -fract(1 / float(size)) * 2.f * Pi));
	for (unsigned i = 2; i < size / 2; ++i) {
		exps[i] = exps[1] * exps[i - 1];
	}
}

void FFT::operator()(const float* in, float* out, std::size_t start, std::size_t offsetMask, bool absModDecomp) {
	if (!offsetMask) offsetMask = maxSize - 1;
	auto size = this->maxSize;
	for (std::size_t i = 0, elemNum0 = 0, elemNum1 = invInc(elemNum0); i < size; i += 2, elemNum0 = invInc(elemNum1), elemNum1 = invInc(elemNum0)) {
		auto a = in[(start + elemNum0) & offsetMask];
		auto b = in[(start + elemNum1) & offsetMask];
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
//		if (i != 0) {
//			auto t = data[size - i] + data[size / 2 - i] * exps[i];
//			a += std::conj(t);
////			a *= 2;
//		}
		if (absModDecomp) {
			out[i * 2] = std::abs(a) / size;
			out[i * 2 + 1] = std::arg(a);
		} else {
			out[i * 2] = a.real() / size;
			out[i * 2 + 1] = a.imag() / size;
		}
	}
}

void FFT::transform(const float* in, float* out, std::size_t start, std::size_t offsetMask, bool unpack)
{
	if (!offsetMask) offsetMask = maxSize - 1;
	auto size = this->maxSize;
	for (std::size_t i = 0, elemNum0 = 0; i < size; i += 1, elemNum0 = invInc(elemNum0)) {
		if (unpack) {
			data[i] = { in[(start + elemNum0) & offsetMask], 0 };
		} else {
			auto rPos = ((start + elemNum0) & offsetMask) * 2;
			auto imPos = rPos + 1;
			data[i] = { in[rPos], in[imPos] };
		}
	}
	for (std::size_t cols = size, rows = 1, expInd = cols / 2; cols != 1; cols = expInd, expInd /= 2, rows *= 2) {
		for (std::size_t col = 0; col < cols; col += 2) {
			for (std::size_t row = 0; row < rows; ++row) {
				auto a = data[calcPos(row, col, rows, cols)];
				auto b = data[calcPos(row, col + 1, rows, cols)];
				auto w = exps[row * expInd];
				data[calcPos(row, col, rows, cols)] = (a + b * w);
				data[calcPos(row, col + 1, rows, cols)] = (a - b * w);
			}
		}
	}
	for (std::size_t i = 0; i < size; ++i) {
		out[i * 2] = data[i].real() / size;
		out[i * 2 + 1] = data[i].imag() / size;
	}
}

void FFT::reverse(const float* in, float* out, std::size_t start, std::size_t offsetMask)
{
	if (!offsetMask) offsetMask = maxSize - 1;
	auto size = this->maxSize;
	for (std::size_t i = 0, elemNum0 = 0; i < size; i += 1, elemNum0 = invInc(elemNum0)) {
		auto rPos = ((start + elemNum0) & offsetMask) * 2;
		auto imPos = rPos + 1;
		data[i] = { in[rPos], in[imPos] };
	}
	for (std::size_t cols = size, rows = 1, expInd = cols / 2; cols != 1; cols = expInd, expInd /= 2, rows *= 2) {
		for (std::size_t col = 0; col < cols; col += 2) {
			for (std::size_t row = 0; row < rows; ++row) {
				auto a = data[calcPos(row, col, rows, cols)];
				auto b = data[calcPos(row, col + 1, rows, cols)];
				auto w = std::conj(exps[row * expInd]);
				data[calcPos(row, col, rows, cols)] = a + b * w;
				data[calcPos(row, col + 1, rows, cols)] = a - b * w;
			}
		}
	}
	for (std::size_t i = 0; i < size; ++i) {
		out[i * 2] = data[i].real();
		out[i * 2 + 1] = data[i].imag();
	}
}

std::size_t FFT::invInc(std::size_t v) {
	return invInc(v, maxSize);
}

std::size_t FFT::invInc(std::size_t v, std::size_t size) {
	std::size_t t = size / 2;
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
