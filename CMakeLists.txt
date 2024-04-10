cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(litho_scheduling)

message("Cmake for project: ${PROJECT_NAME}")
message("Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")


include_directories(include) 

include_directories(/home/arthur/opt/or-tools/include)
link_directories(/home/arthur/opt/or-tools/lib)

add_executable(${PROJECT_NAME} 
        src/main.cpp
        src/read_data.cpp
        src/build_model.cpp
        src/solve_model.cpp
        )

target_link_libraries(${PROJECT_NAME} ortools)

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build
)