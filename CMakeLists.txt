cmake_minimum_required(VERSION 2.8)

PROJECT(libcxi)

if(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99 -W")
endif(CMAKE_COMPILER_IS_GNUCC)

set(CMAKE_C_FLAGS_DEBUG "-DCXI_DEBUG")

find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)
add_library(cxi SHARED src/cxi.c include/cxi.h)
target_link_libraries(cxi ${HDF5_LIBRARIES})

add_executable(simple EXCLUDE_FROM_ALL src/cxi.c tests/simple.c)
target_link_libraries(simple ${HDF5_LIBRARIES})

add_executable(writer EXCLUDE_FROM_ALL src/cxi.c tests/writer.c)
target_link_libraries(writer ${HDF5_LIBRARIES})

add_executable(typical_reader  src/cxi.c examples/typical_reader.c)
target_link_libraries(typical_reader ${HDF5_LIBRARIES})

add_executable(typical_writer  src/cxi.c examples/typical_writer.c)
target_link_libraries(typical_writer ${HDF5_LIBRARIES})

add_executable(minimal_reader  src/cxi.c examples/minimal_reader.c)
target_link_libraries(minimal_reader ${HDF5_LIBRARIES})

add_executable(minimal_writer  src/cxi.c examples/minimal_writer.c)
target_link_libraries(minimal_writer ${HDF5_LIBRARIES})


enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_test(simple simple ${CMAKE_SOURCE_DIR}/data/typical_raw.cxi)

add_test(writer writer ${CMAKE_SOURCE_DIR}/data/dummy.cxi)
add_dependencies(check simple writer)



