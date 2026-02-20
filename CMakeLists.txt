cmake_minimum_required(VERSION 4.0)
project(rpy_proj_analyzer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set_property(TARGET rpy_proj_analyzer PROPERTY CXX_STANDARD 23)
#set_property(TARGET rpy_proj_analyzer PROPERTY CXX_STANDARD_REQUIRED ON)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
    endif()
endif()

add_executable(rpy_proj_analyzer
        include/raylib-cpp.hpp
        src/main.cpp
        src/Lexer.cpp
        src/Lexer.hpp
        src/Token.cpp
        src/Token.hpp
        src/Node.cpp
        src/Node.hpp
        src/Graph.cpp
        src/Graph.hpp
        src/Expr.cpp
        src/Expr.hpp
        src/DisplayNode.cpp
        src/DisplayNode.hpp
        src/TextHelper.cpp
        src/TextHelper.hpp
        src/GraphLayout.cpp
        src/GraphLayout.hpp
        src/ArgVParser.cpp
        src/ArgVParser.hpp
        src/App.cpp
        src/App.hpp
        src/Screen.cpp
        src/Screen.hpp
)

target_include_directories(rpy_proj_analyzer PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(rpy_proj_analyzer raylib)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
