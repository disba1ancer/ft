/*
 * int24_t.h
 *
 *  Created on: 12 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef INT24_T_H_
#define INT24_T_H_

#include <memory>

namespace ft {

template <std::size_t bytes>
class byte_aligned_int {
public:
	byte_aligned_int() = default;
	byte_aligned_int(std::uint32_t v) {
		for (int i = 0; i < bytes; ++i) {
			data[i] = v & ((1 << CHAR_BIT) - 1);
			v >>= CHAR_BIT;
		}
	}
private:
	unsigned char data[bytes];
};

typedef byte_aligned_int<3> int24_t;

} // namespace ft

#endif /* INT24_T_H_ */
