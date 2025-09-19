cmake_minimum_required(VERSION 3.15)
project(MyDrogonApp)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Conan: add path to generated cmake config
#list(APPEND CMAKE_PREFIX_PATH "build/Release/generators")
list(APPEND CMAKE_PREFIX_PATH "/home/vagrant/cpptest/build/Release/generators")
# Find Drogon (provided by CMakeDeps)
find_package(Drogon REQUIRED)
find_package(Boost REQUIRED)
find_package(Curses REQUIRED)
# Add executable
add_executable(my_server main.cpp matching_engine.hpp matching_engine.cpp )

# Link with Drogon
target_link_libraries(my_server  boost::boost ncurses)
