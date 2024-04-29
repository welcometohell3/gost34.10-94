cmake_minimum_required(VERSION 3.8)
project(ds_gost)

set(CMAKE_CXX_STANDARD 11)

find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARY NAMES gmp)
find_library(GMPXX_LIBRARY NAMES gmpxx)

include_directories(${GMP_INCLUDE_DIR})

set(SOURCE_FILES main.cpp sig.hpp sha1.hpp LinearGenerator.h)
add_executable(ds_gost ${SOURCE_FILES})
target_link_libraries(ds_gost ${GMP_LIBRARY} ${GMPXX_LIBRARY})