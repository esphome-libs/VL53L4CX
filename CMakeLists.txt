file(GLOB VL53L4CX_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

if(ESP_PLATFORM)
  idf_component_register(SRCS ${VL53L4CX_SOURCES} INCLUDE_DIRS "src")
  return()
endif()

# Host build: compiles the driver without any framework and runs a smoke test against a fake platform interface
cmake_minimum_required(VERSION 3.20)
project(vl53l4cx CXX)
enable_testing()
add_executable(vl53l4cx_host_test ${VL53L4CX_SOURCES} tests/host_test.cpp)
target_include_directories(vl53l4cx_host_test PRIVATE src)
target_compile_features(vl53l4cx_host_test PRIVATE cxx_std_17)
target_compile_options(vl53l4cx_host_test PRIVATE -Wall -Wextra -Wno-unused-parameter -Werror
                                                  -fsanitize=address,undefined -fno-sanitize-recover=all)
target_link_options(vl53l4cx_host_test PRIVATE -fsanitize=address,undefined)
add_test(NAME vl53l4cx_host_test COMMAND vl53l4cx_host_test)
