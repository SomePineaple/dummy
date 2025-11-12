cmake_minimum_required(VERSION 4.0)
project(dummy)

set(CMAKE_CXX_STANDARD 20)

add_executable(dummy src/main.cpp
        src/game/cards.cpp
        src/game/cards.h
        src/game/meld.cpp
        src/game/meld.h)
