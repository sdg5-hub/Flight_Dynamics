cmake_minimum_required(VERSION 3.20)
project(FlightDynamics VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(FLIGHTLAB_ENABLE_JSBSIM "Build the pinned JSBSim backend" ON)

set(FLIGHTLAB_SOURCES
  src/control/autopilot.cpp
  src/core/metrics.cpp
  src/core/scenario_runner.cpp
  src/backends/trainer_dynamics_backend.cpp
  src/telemetry/csv_recorder.cpp)

if(FLIGHTLAB_ENABLE_JSBSIM)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsbsim/CMakeLists.txt")
    message(FATAL_ERROR "JSBSim submodule missing. Clone with --recurse-submodules or configure -DFLIGHTLAB_ENABLE_JSBSIM=OFF.")
  endif()
  # Keep the vendored project limited to its C++ library.
  set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(BUILD_PYTHON_MODULE OFF CACHE BOOL "" FORCE)
  set(BUILD_JULIA_PACKAGE OFF CACHE BOOL "" FORCE)
  add_subdirectory(third_party/jsbsim EXCLUDE_FROM_ALL)
  list(APPEND FLIGHTLAB_SOURCES src/backends/jsbsim_backend.cpp)
  # JSBSim's public headers are compiled into this thin adapter translation unit.
  # Keep third-party-header diagnostics from obscuring FlightLab diagnostics elsewhere.
  set_source_files_properties(src/backends/jsbsim_backend.cpp PROPERTIES COMPILE_OPTIONS "-w")
endif()

add_library(flightlab_core ${FLIGHTLAB_SOURCES})
target_include_directories(flightlab_core PUBLIC include)
target_compile_options(flightlab_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
if(FLIGHTLAB_ENABLE_JSBSIM)
  target_link_libraries(flightlab_core PUBLIC libJSBSim)
  target_compile_definitions(flightlab_core PUBLIC FLIGHTLAB_HAS_JSBSIM=1
    FLIGHTLAB_DEFAULT_JSBSIM_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsbsim")
endif()

add_executable(flightlab_run apps/flightlab_run.cpp)
target_link_libraries(flightlab_run PRIVATE flightlab_core)

enable_testing()
add_executable(flightlab_tests tests/flightlab_tests.cpp)
target_link_libraries(flightlab_tests PRIVATE flightlab_core)
add_test(NAME flightlab_tests COMMAND flightlab_tests)
