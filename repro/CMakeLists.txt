# CMakeLists.txt
#cmake_minimum_required(VERSION 3.28)
#project(repro CXX)
#set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_SCAN_FOR_MODULES ON)

add_library(repro_mod)
target_sources(repro_mod PUBLIC FILE_SET CXX_MODULES FILES mod.cppm)
set_target_properties(repro_mod PROPERTIES CXX_MODULE_STD ON)

target_compile_features(repro_mod PUBLIC cxx_std_23)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE repro_mod)
set_target_properties(main PROPERTIES CXX_MODULE_STD ON)

target_compile_features(main PUBLIC cxx_std_23)
