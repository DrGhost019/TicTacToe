cmake_minimum_required(VERSION 3.20)
project(TicTacToe2026 VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TTT_CLI_ONLY "Build standalone CLI version without Qt6 GUI" OFF)
option(TTT_BUILD_TESTS "Build the engine verification suite" ON)

add_library(ttt_engine STATIC
    src/engine/Board.cpp
    src/engine/AI.cpp
    src/engine/GameSession.cpp
)
target_include_directories(ttt_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(ttt_engine PRIVATE /W4 /O2)
else()
    target_compile_options(ttt_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(NOT TTT_CLI_ONLY)
    find_package(Qt6 COMPONENTS Widgets Gui Core QUIET)
    if(Qt6_FOUND)
        set(CMAKE_AUTOMOC ON)
        add_executable(TicTacToe
            src/main.cpp
            src/cli/ConsoleRunner.cpp
            src/gui/BoardWidget.cpp
            src/gui/MainWindow.cpp
        )
        target_link_libraries(TicTacToe PRIVATE ttt_engine Qt6::Widgets Qt6::Gui Qt6::Core)
    else()
        message(STATUS "Qt6 not found — falling back to CLI-only build.")
        set(TTT_CLI_ONLY ON)
    endif()
endif()

if(TTT_CLI_ONLY)
    add_executable(TicTacToe
        src/main.cpp
        src/cli/ConsoleRunner.cpp
    )
    target_compile_definitions(TicTacToe PRIVATE TTT_CLI_ONLY)
    target_link_libraries(TicTacToe PRIVATE ttt_engine)
endif()

if(TTT_BUILD_TESTS)
    enable_testing()
    add_executable(test_engine tests/test_engine.cpp)
    target_link_libraries(test_engine PRIVATE ttt_engine)
    add_test(NAME EngineVerificationSuite COMMAND test_engine)
endif()
