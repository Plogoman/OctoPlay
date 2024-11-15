cmake_minimum_required(VERSION 3.29)
project(OctoPlay)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)

add_executable(OctoPlay main.cpp
        Chip8.cpp
        Chip8.h
        GUI.cpp
        GUI.h
        Type.h
)

target_link_libraries(OctoPlay PRIVATE glad::glad glfw imgui::imgui)