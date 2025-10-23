
cmake_minimum_required(VERSION 3.20)
project(AnalyzeInstrPass)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})

add_llvm_library(AnalyzeInstrPass
    AnalyzeInstrPass.cpp
    MODULE
)
