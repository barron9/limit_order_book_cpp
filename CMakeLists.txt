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
