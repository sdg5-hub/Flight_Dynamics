cmake_minimum_required(VERSION 3.20)
project(FlightLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(flightlab_core
  src/core/scenario_runner.cpp
  src/backends/steady_flight_backend.cpp
  src/telemetry/csv_recorder.cpp)
target_include_directories(flightlab_core PUBLIC include)
target_compile_options(flightlab_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)

add_executable(flightlab_run apps/flightlab_run.cpp)
target_link_libraries(flightlab_run PRIVATE flightlab_core)

enable_testing()
add_executable(flightlab_tests tests/flightlab_tests.cpp)
target_link_libraries(flightlab_tests PRIVATE flightlab_core)
add_test(NAME flightlab_tests COMMAND flightlab_tests)
