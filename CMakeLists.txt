cmake_minimum_required(VERSION 3.27)
project(hw1)

set(CMAKE_CXX_STANDARD 23)

add_executable(hw1 main.cpp
        decodingHashMaps.cpp
        decodingHashMaps.h
        instructionDecoding.cpp
        instructionDecoding.h
        byteReader.cpp
        byteReader.h
        registerState.cpp
        registerState.h)
