/*
 * move_only.h
 *
 *  Created on: 3 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_MOVE_ONLY_H_
#define WIN32_MOVE_ONLY_H_

#include <utility>

namespace win32 {

template <typename Rs>
class move_only {
public:
	operator Rs() const {
		return resource;
	}
	const Rs* get_ptr() const { return &resource; }
protected:
	move_only(Rs res) : resource(res){}
	move_only(const move_only&) = delete;
	move_only(move_only&& other) : resource(other.resource) {
		other.resource = 0;
	}
	move_only& operator=(const move_only&) = delete;
	move_only& operator=(move_only&& other) {
		std::swap(resource, other.resource);
		return *this;
	}
	Rs resource;
};

}

#endif /* WIN32_MOVE_ONLY_H_ */
