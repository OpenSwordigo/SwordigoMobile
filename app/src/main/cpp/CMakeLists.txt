cmake_minimum_required(VERSION 3.22.1)

project("swd")

# Support C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    swd
    SHARED
    swd_main.cpp
    swd_jni_bridge.cpp
    swd_launcher_port.cpp
)

find_library(log-lib log)
find_library(gles2-lib GLESv2)
find_library(gles1-lib GLESv1_CM)
find_library(egl-lib EGL)
find_library(android-lib android)
find_library(opensl-lib OpenSLES)

target_link_libraries(
    swd
    ${log-lib}
    ${gles2-lib}
    ${gles1-lib}
    ${egl-lib}
    ${android-lib}
    ${opensl-lib}
    dl
)
