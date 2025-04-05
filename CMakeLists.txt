cmake_minimum_required(VERSION 3.27.5)

project(huedra)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/*.cpp 
)

list(FILTER SOURCES EXCLUDE REGEX "${PROJECT_SOURCE_DIR}/huedra/platform/.*")

add_executable(huedra ${SOURCES})

# Configure Platform
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  message(STATUS "Building for Windows")
  target_compile_definitions(huedra PRIVATE -DWIN32 -DNOMINMAX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message(STATUS "Building for Mac")
  target_compile_definitions(huedra PRIVATE -DMACOS)
endif()

# Configure platform specific libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  find_package(Vulkan REQUIRED)
  include_directories(huedra {Vulkan_INCLUDE_DIRS})
  target_link_libraries(huedra PRIVATE Vulkan::Vulkan)
  target_link_libraries(huedra PRIVATE $ENV{VULKAN_SDK}/Lib/slang.lib)
  target_compile_definitions(huedra PRIVATE -DVULKAN)

  file(GLOB_RECURSE ADDITIONAL_SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/platform/win32/*.cpp 
    ${PROJECT_SOURCE_DIR}/huedra/platform/vulkan/*.cpp 
    ${PROJECT_SOURCE_DIR}/huedra/platform/slang/*.cpp 
  )
  target_sources(huedra PRIVATE ${ADDITIONAL_SOURCES})

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  find_library(COCOA_LIBRARY Cocoa REQUIRED)
  find_library(METAL_LIBRARY Metal REQUIRED)
  find_library(METAL_KIT_LIBRARY MetalKit REQUIRED)
  target_compile_definitions(huedra PRIVATE -DCOCOA)
  target_compile_definitions(huedra PRIVATE -DMETAL)

  target_link_libraries(${PROJECT_NAME} 
      ${COCOA_LIBRARY}
      ${METAL_LIBRARY}
      ${METAL_KIT_LIBRARY}
  )

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++")

  file(GLOB_RECURSE ADDITIONAL_SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/platform/cocoa/*.cpp
    ${PROJECT_SOURCE_DIR}/huedra/platform/cocoa/*.mm
    ${PROJECT_SOURCE_DIR}/huedra/platform/metal/*.cpp
    ${PROJECT_SOURCE_DIR}/huedra/platform/metal/*.mm
  )
  target_sources(huedra PRIVATE ${ADDITIONAL_SOURCES})
endif()

target_include_directories(huedra PRIVATE
  ${PROJECT_SOURCE_DIR}/huedra
)

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