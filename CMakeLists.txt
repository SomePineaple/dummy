cmake_minimum_required(VERSION 3.26.5)
project(dummy)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(deps/njhomann/json)
add_subdirectory(deps/boostorg/boost)

#find_package(Boost REQUIRED COMPONENTS process)

add_executable(dummy src/main.cpp
        src/game/cards.cpp
        src/game/cards.h
        src/game/meld.cpp
        src/game/meld.h
        src/game/player.cpp
        src/game/player.h
        src/game/game.cpp
        src/game/game.h
        src/game/humanplayer.cpp
        src/game/humanplayer.h
        src/signalbot/signalplayer.cpp
        src/signalbot/signalplayer.h)

target_link_libraries(dummy PRIVATE nlohmann_json boost_process boost_asio boost_format)
target_include_directories(dummy PRIVATE deps/njhomann/json/include deps/boostorg/process/include deps/boostorg/format/include)
