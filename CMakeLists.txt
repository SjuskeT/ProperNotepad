cmake_minimum_required(VERSION 3.21)

project(ProperNotepad VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

qt_add_executable(ProperNotepad
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/editortab.cpp
    src/editortab.h
)

target_link_libraries(ProperNotepad PRIVATE Qt6::Widgets)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ProperNotepad PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(GNUInstallDirs)
install(TARGETS ProperNotepad RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
