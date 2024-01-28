cmake_minimum_required(VERSION 3.5)

project(ft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(swal)

set(FT_GENERATED_DIR ${PROJECT_BINARY_DIR}/generated)
set(FT_SHADER_OBJECT ${FT_GENERATED_DIR}/shaders/gl31)

set(FT_BUILT_IN_SHADERS
    src/vert.hlsl
    src/frag.hlsl
)

add_custom_command(OUTPUT
    ${FT_SHADER_OBJECT}.cpp
COMMAND
    ${CMAKE_COMMAND}
    -DOUTPUT=${FT_SHADER_OBJECT}
    -P src/pack_shaders_to_cpp.cmake ${FT_BUILT_IN_SHADERS}
DEPENDS
    src/pack_shaders_to_cpp.cmake
    ${FT_BUILT_IN_SHADERS}
BYPRODUCTS
    ${FT_SHADER_OBJECT}.h
WORKING_DIRECTORY
    ${PROJECT_SOURCE_DIR}
)

add_custom_target(shaders SOURCES src/pack_shaders_to_cpp.cmake ${FT_BUILT_IN_SHADERS})

set(SRCS
    ${FT_SHADER_OBJECT}.cpp
    src/App.cpp
    src/App.h
    src/extmath.cpp
    src/extmath.h
    src/ft.cpp
    src/ft.h
    src/main.cpp
    src/win32/initguid.cpp
    src/win32/win32.h
)

add_executable(ft ${SRCS})
target_include_directories(ft PRIVATE ${FT_GENERATED_DIR})
target_compile_definitions(ft PRIVATE UNICODE=1 _WIN32_WINNT=0xA00)
target_link_libraries(ft swal::swal d2d1 d3d11 dxgi d3dcompiler)
