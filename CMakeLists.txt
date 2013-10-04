cmake_minimum_required(VERSION 2.8)

PROJECT(libcxi)

if(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")
endif(CMAKE_COMPILER_IS_GNUCC)

find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)
add_library(cxi SHARED src/cxi.c)
target_link_libraries(cxi ${HDF5_LIBRARIES})

add_executable(simple EXCLUDE_FROM_ALL src/cxi.c tests/simple.c)
target_link_libraries(simple ${HDF5_LIBRARIES})


enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_test(simple simple ${CMAKE_SOURCE_DIR}/data/typical_raw.cxi)
add_dependencies(check simple)



