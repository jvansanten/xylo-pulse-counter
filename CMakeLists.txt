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

add_executable(pulse_counter
  cycfx2/cycfx2prog/pulse_counter.cpp
  cycfx2/cycfx2prog/cycfx2dev.cc
  )
target_link_libraries(pulse_counter ${LIBUSB_LIBRARY})

find_program(SDCC sdcc)
if(SDCC)
  message(STATUS "+ sdcc: ${SDCC}")
  add_custom_target(xylo_setup.ihx
    COMMAND make xylo_setup.ihx && make tidy && mv xylo_setup.ihx ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cycfx2/saxo_loader
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cycfx2/saxo_loader/xylo_setup.c
  )
  add_dependencies(pulse_counter xylo_setup.ihx)
endif()

FIND_PROGRAM(PYTHON_EXECUTABLE python
      PATHS ENV PATH         # look in the PATH environment variable
      NO_DEFAULT_PATH        # do not look anywhere else...
      )

IF(PYTHON_EXECUTABLE)
  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -V
    OUTPUT_VARIABLE STDOUT_VERSION
    ERROR_VARIABLE PYTHON_VERSION
    ERROR_STRIP_TRAILING_WHITESPACE)
  
  IF(STDOUT_VERSION MATCHES "Python")
    set(PYTHON_VERSION "${STDOUT_VERSION}")
  ENDIF()
  
  STRING(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.?([0-9]*)"
    PYTHON_STRIPPED_VERSION
    ${PYTHON_VERSION})
  STRING(REGEX MATCH "([0-9]+)\\.([0-9]+)"
    PYTHON_STRIPPED_MAJOR_MINOR_VERSION
    ${PYTHON_VERSION})
  
  STRING(REPLACE "." "" PYTHON_VERSION_NO_DOTS ${PYTHON_STRIPPED_MAJOR_MINOR_VERSION})
  
  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; sys.stdout.write(sys.prefix)"
                  OUTPUT_VARIABLE PYTHON_ROOT)

  FIND_LIBRARY(PYTHON_LIBRARY
    NAMES python${PYTHON_VERSION_NO_DOTS} python${PYTHON_STRIPPED_MAJOR_MINOR_VERSION}
    PATHS ${PYTHON_ROOT} ${PYTHON_ROOT}/lib
    PATH_SUFFIXES lib
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_DEFAULT_PATH
  )
  FIND_PATH(PYTHON_INCLUDE_DIR
    NAMES Python.h
    PATHS
      ${PYTHON_ROOT}/include
    PATH_SUFFIXES
      python${PYTHON_STRIPPED_MAJOR_MINOR_VERSION}
    NO_DEFAULT_PATH
  )

  IF(EXISTS "${PYTHON_INCLUDE_DIR}/Python.h" )
    SET(PYTHON_FOUND TRUE CACHE BOOL "Python found successfully" FORCE)
    MESSAGE(STATUS "Python found")
    MESSAGE (STATUS "  * binary:   ${PYTHON_EXECUTABLE}")
    MESSAGE (STATUS "  * version:  ${PYTHON_VERSION}")
    MESSAGE (STATUS "  * includes: ${PYTHON_INCLUDE_DIR}")
    MESSAGE (STATUS "  * libs:     ${PYTHON_LIBRARY}")
  ELSE()
    SET(PYTHON_FOUND FALSE CACHE BOOL "Python found successfully" FORCE)
  ENDIF()

  IF(PYTHON_FOUND)
    # Search for numpy
    EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import numpy; print numpy.__version__"
      RESULT_VARIABLE NUMPY_FOUND OUTPUT_VARIABLE NUMPY_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
#    IF(NUMPY_FOUND EQUAL 0 AND NOT NUMPY_VERSION VERSION_LESS 1.7)
#      SET(NUMPY_FOUND TRUE)
#    ELSEIF(NUMPY_VERSION VERSION_LESS 1.7)
#      SET(NUMPY_FOUND FALSE)
#      MESSAGE (STATUS "  x numpy:    (version ${NUMPY_VERSION} < 1.7 is too old)")
    IF(NUMPY_FOUND EQUAL 0)
      SET(NUMPY_FOUND TRUE)
    ELSE()
      SET(NUMPY_FOUND FALSE)
    ENDIF()
  
    IF(NUMPY_FOUND)
      SET(NUMPY_FOUND TRUE CACHE BOOL "Numpy found successfully" FORCE)
      EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c
        "import numpy; print numpy.get_include()"
        OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
      SET(NUMPY_INCLUDE_DIR ${NUMPY_INCLUDE_DIR} CACHE STRING "Numpy directory")
      MESSAGE (STATUS "  * numpy:    version ${NUMPY_VERSION}; headers ${NUMPY_INCLUDE_DIR}")
    ELSE()
      SET(NUMPY_FOUND FALSE CACHE BOOL "Numpy found successfully" FORCE)
    ENDIF()
  ENDIF(PYTHON_FOUND)
ENDIF(PYTHON_EXECUTABLE)

IF(PYTHON_FOUND)
  MESSAGE("-- Will build the python module")
  ADD_LIBRARY (pybindings SHARED
    cycfx2/cycfx2prog/pypulse_counter.cpp
    cycfx2/cycfx2prog/cycfx2dev.cc
  )
  SET_TARGET_PROPERTIES(pybindings PROPERTIES LINKER_LANGUAGE C OUTPUT_NAME "pulse_counter" PREFIX "" SUFFIX ".so")
  if(APPLE)
    SET_TARGET_PROPERTIES(pybindings PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  endif(APPLE)
  target_link_libraries(pybindings ${LIBUSB_LIBRARY})
  target_include_directories(pybindings PRIVATE ${PYTHON_INCLUDE_DIR})
ENDIF()
