cmake_minimum_required(VERSION 3.26.5)
project(dummy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
            "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

add_subdirectory(deps/njhomann/json)
add_subdirectory(deps/boostorg/boost)

add_subdirectory(deps/davisking/dlib)

add_library(dummy_backend src/game/cards.cpp
        src/game/cards.h
        src/game/meld.cpp
        src/game/meld.h
        src/game/clients/player.cpp
        src/game/clients/player.h
        src/game/game.cpp
        src/game/game.h
        src/game/clients/console_player.cpp
        src/game/clients/console_player.h
        src/nn/dlib_helper.h
        src/nn/nn_logic.cpp
        src/nn/nn_logic.h
        src/nn/nn_player.cpp
        src/nn/nn_player.h
        src/game/clients/rule_bot.cpp
        src/game/clients/rule_bot.h
        src/game/utils/legal_move_engine.cpp
        src/game/utils/legal_move_engine.h
        src/nn/training/cpu_trainer.cpp
        src/nn/training/cpu_trainer.h
)

target_link_libraries(dummy_backend PRIVATE boost_format dlib::dlib)
target_include_directories(dummy_backend PRIVATE deps/boostorg/format/include deps/Compaile/ctrack PUBLIC deps/davisking/dlib/)

# Enable profiling on debug builds
target_compile_definitions(dummy_backend PRIVATE
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:PROFILE>
)

add_executable(signal-client src/signalclient.cpp
        src/signalbot/signal_player.cpp
        src/signalbot/signal_player.h)

target_link_libraries(signal-client PRIVATE dummy_backend nlohmann_json boost_process boost_asio boost_format)
target_include_directories(signal-client PRIVATE deps/njhomann/json/include deps/boostorg/process/include deps/boostorg/format/include)

add_executable(console-client src/consoleclient.cpp)

target_link_libraries(console-client PRIVATE dummy_backend)

add_executable(ai-trainer src/aitrainer.cpp)
target_include_directories(ai-trainer PRIVATE deps/boostorg/libs/asio/include)
target_link_libraries(ai-trainer PRIVATE dummy_backend dlib::dlib Boost::asio)

# Testing
add_executable(dummy-tester test/test.cpp)
target_include_directories(dummy-tester PRIVATE deps/boostorg/boost/libs/test/include src/ deps/Compaile/ctrack)
target_link_libraries(dummy-tester PRIVATE Boost::unit_test_framework dummy_backend)

target_compile_definitions(dummy-tester PRIVATE
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:PROFILE>
)

add_test(NAME testStuff COMMAND dummy-tester)
