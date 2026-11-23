# Reads CMakeLists.txt the way ESP-IDF's requirements scan does: in script mode, with ESP_PLATFORM set
# Run with: cmake -DCOMPONENT_DIR=<repo> -P tests/idf_requirements_scan.cmake
set(ESP_PLATFORM 1)
set(CMAKE_CURRENT_SOURCE_DIR ${COMPONENT_DIR})
function(idf_component_register)
  message(STATUS "idf_component_register ${ARGN}")
endfunction()
include(${COMPONENT_DIR}/CMakeLists.txt)
