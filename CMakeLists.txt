cmake_minimum_required(VERSION 3.1.0)

project(PulseCounter C CXX)
set(CMAKE_CXX_STANDARD 14)


find_path(LIBUSB_INCLUDE_DIR usb.h)
find_library(LIBUSB_LIBRARY usb)
set(LIBUSB_FOUND LIBUSB_INCLUDE_DIR AND LIBUSB_LIBRARY)
if (LIBUSB_FOUND)
  message(STATUS "+ libusb")
  message(STATUS "  include: ${LIBUSB_INCLUDE_DIR}")
  message(STATUS "      lib: ${LIBUSB_LIBRARY}")
else()
  message(FATAL_ERROR "Couldn't find libusb-0.1")
endif ()

include_directories(${LIBUSB_INCLUDE_DIR})

add_executable(cycfx2prog
  cycfx2/cycfx2prog/cycfx2prog.cc
  cycfx2/cycfx2prog/cycfx2dev.cc
)

target_compile_definitions(cycfx2prog PRIVATE -DCYCFX2PROG_VERSION=\"0.47\")
target_link_libraries(cycfx2prog ${LIBUSB_LIBRARY})

add_executable(pulse-counter
  cycfx2/cycfx2prog/pulse_counter.cpp
  cycfx2/cycfx2prog/cycfx2dev.cc
  )
target_link_libraries(pulse-counter ${LIBUSB_LIBRARY})
target_include_directories(pulse-counter PRIVATE ${LIBUSB_INCLUDE_DIR})

find_program(SDCC sdcc)
if(SDCC)
  message(STATUS "+ sdcc: ${SDCC}")
  add_custom_target(xylo_setup.ihx
    COMMAND make xylo_setup.ihx && make tidy && mv xylo_setup.ihx ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cycfx2/saxo_loader
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cycfx2/saxo_loader/xylo_setup.c
  )
  add_dependencies(pulse-counter xylo_setup.ihx)
endif()

find_package(Python COMPONENTS Interpreter Development)
IF(Python_FOUND)
  MESSAGE(STATUS "Python found")
  MESSAGE (STATUS "  * binary:   ${Python_EXECUTABLE}")
  MESSAGE (STATUS "  * version:  ${Python_VERSION}")
  MESSAGE (STATUS "  * includes: ${Python_INCLUDE_DIR}")
  MESSAGE("-- Will build the python module")
  Python_add_library(pulse_counter MODULE
    cycfx2/cycfx2prog/pypulse_counter.cpp
    cycfx2/cycfx2prog/cycfx2dev.cc
  )
  target_link_libraries(pulse_counter PRIVATE ${LIBUSB_LIBRARY})
  install(TARGETS pulse_counter DESTINATION ${Python_SITEARCH})
ENDIF()
