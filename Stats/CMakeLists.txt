cmake_minimum_required(VERSION 3.0)

project(PulseCounter CXX)

find_path(WIN32_INC windows.h)
set(CyAPI_ROOT "C:/Cypress/Cypress Suite USB 3.4.7/CyAPI")

include_directories(${CyAPI_ROOT}/inc ${WIN32_INC})
link_directories(${CyAPI_ROOT}/lib/x64)

add_library(Stats SHARED StatsDLL.cpp)
target_link_libraries(Stats CyAPI SetupAPI legacy_stdio_definitions)

add_executable(pulse_counter pulse_counter.cpp)
target_link_libraries(pulse_counter Stats)
