cmake_minimum_required(VERSION 3.27.5)

include(${CMAKE_SOURCE_DIR}/cmake/dependencies.cmake)

project(huedra)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/*.cpp 
)

list(FILTER SOURCES EXCLUDE REGEX "${PROJECT_SOURCE_DIR}/huedra/platform/.*")

add_executable(huedra ${SOURCES})

# Configure platforms and specific libraries
if(WIN32)
  message(STATUS "Building for Windows")
  target_compile_definitions(huedra PRIVATE -DWIN32 -DNOMINMAX)

  find_package(Vulkan REQUIRED)
  include_directories(huedra {Vulkan_INCLUDE_DIRS})
  target_link_libraries(huedra PRIVATE Vulkan::Vulkan)
  target_compile_definitions(huedra PRIVATE -DVULKAN)

  file(GLOB_RECURSE ADDITIONAL_SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/platform/win32/*.cpp 
    ${PROJECT_SOURCE_DIR}/huedra/platform/vulkan/*.cpp 
    ${PROJECT_SOURCE_DIR}/huedra/platform/slang/*.cpp 
  )
  target_sources(huedra PRIVATE ${ADDITIONAL_SOURCES})

elseif(APPLE)
  message(STATUS "Building for MacOS")
  target_compile_definitions(huedra PRIVATE -DMACOS)

  find_library(COCOA_LIBRARY Cocoa REQUIRED)
  find_library(METAL_LIBRARY Metal REQUIRED)
  find_library(METAL_KIT_LIBRARY MetalKit REQUIRED)
  find_library(QUARTZ_CORE QuartzCore REQUIRED)
  target_compile_definitions(huedra PRIVATE -DCOCOA)
  target_compile_definitions(huedra PRIVATE -DMETAL)

  target_link_libraries(huedra PRIVATE
      ${COCOA_LIBRARY}
      ${METAL_LIBRARY}
      ${METAL_KIT_LIBRARY}
      ${QUARTZ_CORE}
  )

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++")

  file(GLOB_RECURSE ADDITIONAL_SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/platform/cocoa/*.cpp
    ${PROJECT_SOURCE_DIR}/huedra/platform/cocoa/*.mm
    ${PROJECT_SOURCE_DIR}/huedra/platform/metal/*.cpp
    ${PROJECT_SOURCE_DIR}/huedra/platform/metal/*.mm
    ${PROJECT_SOURCE_DIR}/huedra/platform/slang/*.cpp 
  )
  target_sources(huedra PRIVATE ${ADDITIONAL_SOURCES})
endif()

# Link dependencies
target_link_libraries(huedra PRIVATE slang)

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

# Copy assets
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${source_assets} ${dest_assets}
    COMMENT "Copying assets to build folder"
)

if(WIN32)
  # Copy dependency DLLs
  add_custom_target(copy_dlls ALL
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${DLLS}
          ${PROJECT_BINARY_DIR}
      COMMENT "Copying DLL files to build folder"
  )
  add_dependencies(huedra copy_dlls)
endif()

add_dependencies(huedra copy_assets)