cmake_minimum_required(VERSION 3.26.5)
project(dummy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(deps/njhomann/json)
add_subdirectory(deps/boostorg/boost)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(USE_IMAGE_API OFF)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(deps/tiny-dnn)

add_library(dummy_backend src/game/cards.cpp
        src/game/cards.h
        src/game/meld.cpp
        src/game/meld.h
        src/game/player.cpp
        src/game/player.h
        src/game/game.cpp
        src/game/game.h
        src/game/console_player.cpp
        src/game/console_player.h
        src/nn/nn_logic.cpp
        src/nn/nn_logic.h
        src/nn/nn_player.cpp
        src/nn/nn_player.h)

target_link_libraries(dummy_backend PRIVATE boost_format)
target_include_directories(dummy_backend PRIVATE deps/boostorg/format/include deps/tiny-dnn)

add_executable(signal-client src/signalclient.cpp
        src/signalbot/signal_player.cpp
        src/signalbot/signal_player.h)

target_link_libraries(signal-client PRIVATE dummy_backend nlohmann_json boost_process boost_asio boost_format)
target_include_directories(signal-client PRIVATE deps/njhomann/json/include deps/boostorg/process/include deps/boostorg/format/include)

add_executable(console-client src/consoleclient.cpp)

target_link_libraries(console-client PRIVATE dummy_backend)

add_executable(ai-trainer src/aitrainer.cpp)
target_include_directories(ai-trainer PRIVATE deps/tiny-dnn)
target_link_libraries(ai-trainer PRIVATE dummy_backend)
