cmake_minimum_required(VERSION 3.10)

project(pxDisplay VERSION 1.0 LANGUAGES CXX)

set(EXECUTABLE_NAME px-computers)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainScreen.cpp
    src/ScreenInput.cpp
    src/pxAssemble.cpp
    src/pxCPU.cpp
    src/pxDisplay.cpp
    src/pxMainboard.cpp
    src/pxOpenGL.cpp
)

set(HEADERS
    #
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${EXECUTABLE_NAME} ${SOURCES} ${HEADERS})

set(CMAKE_GENERATOR_PLATFORM x64)

find_package(OpenGL REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} OpenGL32)

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

if (MSVC)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    add_compile_definitions(SUBSYSTEM_CONSOLE)
    #set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    #add_compile_definitions(SUBSYSTEM_WINDOWS)
else()
    target_link_options(${EXECUTABLE_NAME} PRIVATE -mwindows)
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
