cmake_minimum_required(VERSION 3.15)
project(orderbook LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages (provided by Conan's CMakeDeps)
find_package(Boost REQUIRED)
#find_package(pdcurses REQUIRED)
find_package(Curses REQUIRED)
# Define executable
add_executable(my_server
    main.cpp
    matching_engine.cpp
    Session.cpp
    Server.cpp
)
# Link libraries
target_link_libraries(my_server
        boost::boost
        Curses::Curses
)
 
# ---------------------------
# Benchmark tests executable
add_executable(benchmark_tests Bench.cpp)  # Your benchmark source(s)

target_include_directories(benchmark_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/include
)

target_link_libraries(benchmark_tests PRIVATE
    optimized ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/build/src/Release/benchmark.lib
    optimized ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/build/src/Release/benchmark_main.lib
    shlwapi
    advapi32
)

target_compile_definitions(benchmark_tests PRIVATE
    /O2
    BENCHMARK_STATIC_DEFINE
)