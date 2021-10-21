cmake_minimum_required(VERSION 3.15.0)
project(zipline-takehome)

add_compile_options(-Wextra -Wall)

# set C standard
set(CMAKE_C_STANDARD 99)

# add address sanitizers
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

# add library
add_library(pool_alloc STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/pool_alloc.c)
target_include_directories(pool_alloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

set(LIBS pool_alloc)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
