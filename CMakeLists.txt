cmake_minimum_required(VERSION 3.27.5)

project(huedra)

set(CMAKE_CXX_STANDARD 23)

file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/*.cpp 
    ${PROJECT_SOURCE_DIR}/huedra/**/*.cpp 
)

add_executable(huedra ${SOURCES})

target_include_directories(huedra PRIVATE
  ${PROJECT_SOURCE_DIR}/core
)

# Define compiler flags and macros for different build types
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(huedra PRIVATE -DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_definitions(huedra PRIVATE -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  target_compile_definitions(huedra PRIVATE -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  target_compile_definitions(huedra PRIVATE -DNDEBUG)
endif()