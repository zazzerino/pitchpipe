cmake_minimum_required(VERSION 3.21)
project(pitchpipe C)

set(CMAKE_C_STANDARD 17)

add_executable(pitchpipe main.c)
target_link_libraries(pitchpipe m portaudio)