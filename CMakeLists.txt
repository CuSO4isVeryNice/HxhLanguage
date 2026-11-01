cmake_minimum_required(VERSION 3.15)
project(HxhLanguage VERSION 1.14 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# llvm-config 获取所有编译与链接配置
execute_process(COMMAND llvm-config --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIRS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --libs core support native irreader asmparser OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE LLVM_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --version OUTPUT_VARIABLE LLVM_VERSION_STRING OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "LLVM版本为 ${LLVM_VERSION_STRING} 喵～")

# 配置头文件与编译参数
include_directories(${LLVM_INCLUDE_DIRS})
add_compile_options(${LLVM_CXXFLAGS})

add_compile_options(-O3)
add_compile_options(-finline-functions)
add_compile_options(-funroll-loops)

add_executable(hxhc ${CMAKE_SOURCE_DIR}/src/Main.cpp)

# 注入动态链接路径和 LLVM 动态库
target_link_options(hxhc PRIVATE ${LLVM_LDFLAGS})
target_link_libraries(hxhc 
    PRIVATE 
        LLVM
)
