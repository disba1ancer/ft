/*
 * extmath.h
 *
 *  Created on: 13 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef EXTMATH_H_
#define EXTMATH_H_

#include <cmath>

namespace ft {

constexpr float Pi = 3.14159265f;//std::acos(-1.f);//

inline /*constexpr*/ float fract(float v) {
	return v - std::floor(v);
}

} // namespace ft

#endif /* EXTMATH_H_ */
