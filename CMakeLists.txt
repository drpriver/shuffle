cmake_minimum_required(VERSION 3.14) # This is a guess
project(Shuffle VERSION 1.0.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

if(WIN32)
add_compile_options("-D_CRT_NONSTDC_NO_DEPRECATE" "-D_CRT_SECURE_NO_WARNINGS")
elseif(APPLE)
elseif(UNIX)
set(LIBM_LIBRARIES m)
endif()

add_executable(shuffle shuffle.c)
target_link_libraries(shuffle ${LIBM_LIBRARIES})
install(TARGETS shuffle DESTINATION bin)
