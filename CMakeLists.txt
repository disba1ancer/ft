cmake_minimum_required(VERSION 3.5)

project(ft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRCS
    src/App.cpp
    src/App.h
    src/extmath.cpp
    src/extmath.h
    src/ft.cpp
    src/ft.h
    src/main.cpp
    src/win32/com.h
    src/win32/enum_bitwise.h
    src/win32/error.h
    src/win32/gdi.h
    src/win32/handle.h
    src/win32/move_only.h
    src/win32/strconv.h
    src/win32/win32.h
    src/win32/window.h
)

add_executable(ft ${SRCS})
target_compile_definitions(ft PRIVATE UNICODE=1)
target_link_libraries(ft d2d1)
