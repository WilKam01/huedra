cmake_minimum_required(VERSION 3.27.5)

include(FindPackageHandleStandardArgs)
include(${CMAKE_SOURCE_DIR}/cmake/utils.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependencies.cmake)

project(huedra)

set(CMAKE_CXX_STANDARD 23)

append_glob_recurse_files(SOURCES 
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

  append_glob_recurse_files(ADDITIONAL_SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/platform/win32/*.cpp 
    ${PROJECT_SOURCE_DIR}/huedra/platform/vulkan/*.cpp 
  )

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
  
  append_glob_recurse_files(ADDITIONAL_SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/platform/cocoa/*.cpp
    ${PROJECT_SOURCE_DIR}/huedra/platform/cocoa/*.mm
    ${PROJECT_SOURCE_DIR}/huedra/platform/metal/*.cpp
    ${PROJECT_SOURCE_DIR}/huedra/platform/metal/*.mm
  )
  set_source_files_properties(${ADDITIONAL_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++")

elseif(LINUX)
  message(STATUS "Building for Linux")
  target_compile_definitions(huedra PRIVATE -DLINUX)

  find_package(Vulkan REQUIRED)
  include_directories(huedra {Vulkan_INCLUDE_DIRS})
  target_link_libraries(huedra PRIVATE Vulkan::Vulkan)
  target_compile_definitions(huedra PRIVATE -DVULKAN)
  append_glob_recurse_files(ADDITIONAL_SOURCES 
    ${PROJECT_SOURCE_DIR}/huedra/platform/vulkan/*.cpp 
  )

  if($ENV{XDG_SESSION_TYPE} STREQUAL "wayland")
    message(STATUS "Using Wayland Display System")

    # Find include file and lib
    find_path(WAYLAND_INCLUDE_DIR
      NAMES wayland-client.h
      PATHS /usr/include /usr/local/include
    )
    find_library(WAYLAND_LIBRARY
      NAMES wayland-client
      PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
    )
    find_package_handle_standard_args(Wayland 
      REQUIRED_VARS WAYLAND_LIBRARY WAYLAND_INCLUDE_DIR
    )

    if(NOT Wayland_FOUND)
      message(FATAL_ERROR "Could not find Wayland dependencies")
    endif()

    add_library(Wayland::Client UNKNOWN IMPORTED)
    set_target_properties(Wayland::Client PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${WAYLAND_INCLUDE_DIR}"
      IMPORTED_LOCATION "${WAYLAND_LIBRARY}"
    )
    target_link_libraries(huedra PRIVATE Wayland::Client)

    # Find scanner to generate protocol files
    find_program(WAYLAND_SCANNER_EXECUTABLE
      NAMES wayland-scanner
    )

    if(NOT WAYLAND_SCANNER_EXECUTABLE)
      message(FATAL_ERROR "Could not find wayland-scanner to generate protocol files")
    endif()

    find_path(WAYLAND_SHELL_PROTOCOL_DIR
      NAMES stable/xdg-shell/xdg-shell.xml
      PATHS /usr/share/wayland-protocols /usr/local/share/wayland-protocols
    )

    if(NOT WAYLAND_SHELL_PROTOCOL_DIR)
      message(FATAL_ERROR "Could not find wayland shell protocol")
    endif()

    find_path(WAYLAND_DECORATION_PROTOCOL_DIR
      NAMES unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
      PATHS /usr/share/wayland-protocols /usr/local/share/wayland-protocols
    )

    if(NOT WAYLAND_DECORATION_PROTOCOL_DIR)
      message(FATAL_ERROR "Could not find wayland decoration protocol")
    endif()

    # Generate protocol files
    set(SHELL_PROTOCOL_XML "${WAYLAND_SHELL_PROTOCOL_DIR}/stable/xdg-shell/xdg-shell.xml")
    set(SHELL_GENERATED_NAME "${CMAKE_CURRENT_BINARY_DIR}/wayland/xdg-shell")

    set(DECORATION_PROTOCOL_XML "${WAYLAND_DECORATION_PROTOCOL_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml")
    set(DECORATION_GENERATED_NAME "${CMAKE_CURRENT_BINARY_DIR}/wayland/xdg-decoration")

    add_custom_target(gen_wayland_protocol_files
      COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/wayland
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header "${SHELL_PROTOCOL_XML}" "${SHELL_GENERATED_NAME}.h"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code "${SHELL_PROTOCOL_XML}" "${SHELL_GENERATED_NAME}.c"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header "${DECORATION_PROTOCOL_XML}" "${DECORATION_GENERATED_NAME}.h"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code "${DECORATION_PROTOCOL_XML}" "${DECORATION_GENERATED_NAME}.c"
      COMMENT "Generating Wayland Protocol Files..."
    )
    add_dependencies(huedra gen_wayland_protocol_files)

    append_glob_recurse_files(ADDITIONAL_SOURCES 
      ${CMAKE_CURRENT_BINARY_DIR}/wayland/*.c 
      ${PROJECT_SOURCE_DIR}/huedra/platform/wayland/*.cpp 
    )
    target_include_directories(huedra PRIVATE
      ${CMAKE_CURRENT_BINARY_DIR}/wayland
    )

    target_compile_definitions(huedra PRIVATE -DWAYLAND)
    else()
    # Fallback to X11 if available
    if($ENV{XDG_SESSION_TYPE} STREQUAL "x11")
      message(STATUS "Using X11 Display System")
      append_glob_recurse_files(ADDITIONAL_SOURCES 
        ${PROJECT_SOURCE_DIR}/huedra/platform/x11/*.cpp 
      )
      target_compile_definitions(huedra PRIVATE -DX11)
      message(FATAL_ERROR "Currently not supported!")
    else()
      message(FATAL_ERROR "Could not determine Display System")
    endif()
  endif()
endif()

# Link common dependencies
target_link_libraries(huedra PRIVATE slang)

target_include_directories(huedra PRIVATE
  ${PROJECT_SOURCE_DIR}/huedra
)

append_glob_recurse_files(ADDITIONAL_SOURCES 
  ${PROJECT_SOURCE_DIR}/huedra/platform/slang/*.cpp 
)

target_sources(huedra PRIVATE ${ADDITIONAL_SOURCES})

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