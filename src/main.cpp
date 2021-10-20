/*
 * ft.cpp
 *
 *  Created on: 28 авг. 2020 г.
 *      Author: disba1ancer
 */

#include "win32/win32.h"
#include <iostream>
#include "App.h"

int main(int argc, char *argv[]) {
	win32::com_error::throw_or_result(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

	ft::App app(argc, argv);
	return app.run();
}
