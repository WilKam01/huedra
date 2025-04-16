cmake_minimum_required(VERSION 3.27.5)

project(huedra)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/*.cpp 
)

add_executable(huedra ${SOURCES})

find_package(Vulkan REQUIRED)

if(VULKAN_FOUND)
  include_directories(huedra {Vulkan_INCLUDE_DIRS})
  target_link_libraries(huedra PRIVATE Vulkan::Vulkan)
  target_link_libraries(huedra PRIVATE $ENV{VULKAN_SDK}/lib/slang.lib)
  target_compile_definitions(huedra PRIVATE -DVULKAN)
endif(VULKAN_FOUND)

target_include_directories(huedra PRIVATE
  ${PROJECT_SOURCE_DIR}/huedra
)

if(WIN32)
  message(STATUS "Building for Windows")
  set(CMAKE_SYSTEM_NAME Windows)
  target_compile_definitions(huedra PRIVATE -DWIN32 -DNOMINMAX)
endif()

# Define compiler flags and macros for different build types
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(huedra PRIVATE -DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_definitions(huedra PRIVATE -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  target_compile_definitions(huedra PRIVATE -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  target_compile_definitions(huedra PRIVATE -DDEBUG)
endif()

set(source_assets ${PROJECT_SOURCE_DIR}/assets)
set(dest_assets ${PROJECT_BINARY_DIR}/assets)
set(shader_dir ${source_assets}/shaders)

# Copy assets
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${source_assets} ${dest_assets}
    COMMENT "Copying assets to build folder"
)

add_dependencies(huedra copy_assets)