cmake_minimum_required(VERSION 3.14)
project(pwmidicontrol)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files in the src directory
file(GLOB SOURCES "src/*.cpp")

# Add the executable target
add_executable(pwmidicontrol ${SOURCES})

# Include the include/ directory for header files
include_directories(include)

# Link RtMidi to the executable
target_link_libraries(pwmidicontrol PRIVATE rtmidi)
