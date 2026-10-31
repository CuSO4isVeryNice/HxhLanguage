cmake_minimum_required(VERSION 3.15)
project(HxhLanguage VERSION 1.14 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#LLVM-API使用
enable_language(C)
find_package(LLVM)
message(STATUS "LLVM版本为 ${LLVM_PACKAGG_VERSION} 喵～")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_compile_options(-O3)
add_compile_options(-finline-functions)
add_compile_options(-funroll-loops)

add_executable(hxhc ${CMAKE_SOURCE_DIR}/src/Main.cpp)
target_link_libraries(hxhc 
    PRIVATE 
        LLVM
)