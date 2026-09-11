cmake_minimum_required(VERSION 3.15)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(chess_engine
    main.cpp
    board.cpp
    movegen.cpp
    evaluation.cpp
    search.cpp
    play.cpp
    zobrist.cpp
    tt.cpp
)

add_executable(generate_data
    generate_data.cpp
    board.cpp
    movegen.cpp
    evaluation.cpp
    search.cpp
    zobrist.cpp
    tt.cpp
)

add_executable(tune
    tune.cpp
    board.cpp
    movegen.cpp
)