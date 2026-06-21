cmake_minimum_required(VERSION 3.16)
project(bitman_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bitman_core
    src/core/bitman_core.cpp
    src/core/bitman_engine.cpp
    src/core/frame8.cpp
    src/core/motion_classifier.cpp
    src/core/sprites.cpp
)
target_include_directories(bitman_core PUBLIC include)
target_compile_options(bitman_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(bitman_tests tests/test_main.cpp)
target_link_libraries(bitman_tests PRIVATE bitman_core)
target_compile_options(bitman_tests PRIVATE -Wall -Wextra -Wpedantic)

add_executable(bitman_preview tools/preview.cpp)
target_link_libraries(bitman_preview PRIVATE bitman_core)

enable_testing()
add_test(NAME bitman_tests COMMAND bitman_tests)

