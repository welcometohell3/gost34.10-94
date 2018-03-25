cmake_minimum_required(VERSION 3.8)
project(ds_gost)

set(CMAKE_CXX_STANDARD 11)
INCLUDE(cmake/FindGMP.cmake REQUIRED)
include_directories(${GMP_INCLUDE_DIR})

set(SOURCE_FILES main.cpp md5.cpp md5.h Signature.cpp Signature.h LinearGenerator.cpp LinearGenerator.h)
add_executable(ds_gost ${SOURCE_FILES})
target_link_libraries(ds_gost ${GMP_LIBRARIES})