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
    find_path(WAYLAND_CLIENT_INCLUDE_DIR
      NAMES wayland-client.h
      PATHS /usr/include /usr/local/include
    )
    find_library(WAYLAND_CLIENT_LIBRARY
      NAMES wayland-client
      PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
    )
    find_package_handle_standard_args(WAYLAND_CLIENT 
      REQUIRED_VARS WAYLAND_CLIENT_LIBRARY WAYLAND_CLIENT_INCLUDE_DIR
    )

    find_path(WAYLAND_CURSOR_INCLUDE_DIR
      NAMES wayland-cursor.h
      PATHS /usr/include /usr/local/include
    )
    find_library(WAYLAND_CURSOR_LIBRARY
      NAMES wayland-cursor
      PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
    )
    find_package_handle_standard_args(WAYLAND_CURSOR 
      REQUIRED_VARS WAYLAND_CURSOR_LIBRARY WAYLAND_CURSOR_INCLUDE_DIR
    )

    if(NOT WAYLAND_CLIENT_FOUND OR NOT WAYLAND_CURSOR_FOUND)
      message(FATAL_ERROR "Could not find Wayland dependencies")
    endif()

    add_library(Wayland::Client UNKNOWN IMPORTED)
    set_target_properties(Wayland::Client PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${WAYLAND_CLIENT_INCLUDE_DIR}"
      IMPORTED_LOCATION "${WAYLAND_CLIENT_LIBRARY}"
    )
    target_link_libraries(huedra PRIVATE Wayland::Client)

    add_library(Wayland::Cursor UNKNOWN IMPORTED)
    set_target_properties(Wayland::Cursor PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${WAYLAND_CURSOR_INCLUDE_DIR}"
      IMPORTED_LOCATION "${WAYLAND_CURSOR_LIBRARY}"
    )
    target_link_libraries(huedra PRIVATE Wayland::Cursor)

    # Find scanner to generate protocol files
    find_program(WAYLAND_SCANNER_EXECUTABLE
      NAMES wayland-scanner
    )

    if(NOT WAYLAND_SCANNER_EXECUTABLE)
      message(FATAL_ERROR "Could not find wayland-scanner to generate protocol files")
    endif()

    find_path(WAYLAND_PROTOCOLS_DIR
      NAMES wayland-protocols
      PATHS /usr/share/ /usr/local/share/
      NO_DEFAULT_PATH
    )

    if(NOT WAYLAND_PROTOCOLS_DIR)
      message(FATAL_ERROR "Could not find wayland protocols directory")
    endif()

    set(WAYLAND_PROTOCOLS_DIR ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols)

    # Generate protocol files
    set(SHELL_PROTOCOL_XML "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
    set(SHELL_GENERATED_NAME "${CMAKE_CURRENT_BINARY_DIR}/wayland/xdg-shell")

    set(DECORATION_PROTOCOL_XML "${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml")
    set(DECORATION_GENERATED_NAME "${CMAKE_CURRENT_BINARY_DIR}/wayland/xdg-decoration")

    set(WARP_PROTOCOL_XML "${WAYLAND_PROTOCOLS_DIR}/staging/pointer-warp/pointer-warp-v1.xml")
    set(WARP_GENERATED_NAME "${CMAKE_CURRENT_BINARY_DIR}/wayland/pointer-warp")

    set(RELATIVE_POINTER_PROTOCOL_XML "${WAYLAND_PROTOCOLS_DIR}/unstable/relative-pointer/relative-pointer-unstable-v1.xml")
    set(RELATIVE_POINTER_GENERATED_NAME "${CMAKE_CURRENT_BINARY_DIR}/wayland/relative-pointer")

    set(POINTER_CONSTRAINTS_PROTOCOL_XML "${WAYLAND_PROTOCOLS_DIR}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml")
    set(POINTER_CONSTRAINTS_GENERATED_NAME "${CMAKE_CURRENT_BINARY_DIR}/wayland/pointer-constraints")

    execute_process(
      COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/wayland
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header "${SHELL_PROTOCOL_XML}" "${SHELL_GENERATED_NAME}.h"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code "${SHELL_PROTOCOL_XML}" "${SHELL_GENERATED_NAME}.c"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header "${DECORATION_PROTOCOL_XML}" "${DECORATION_GENERATED_NAME}.h"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code "${DECORATION_PROTOCOL_XML}" "${DECORATION_GENERATED_NAME}.c"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header "${WARP_PROTOCOL_XML}" "${WARP_GENERATED_NAME}.h"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code "${WARP_PROTOCOL_XML}" "${WARP_GENERATED_NAME}.c"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header "${RELATIVE_POINTER_PROTOCOL_XML}" "${RELATIVE_POINTER_GENERATED_NAME}.h"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code "${RELATIVE_POINTER_PROTOCOL_XML}" "${RELATIVE_POINTER_GENERATED_NAME}.c"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header "${POINTER_CONSTRAINTS_PROTOCOL_XML}" "${POINTER_CONSTRAINTS_GENERATED_NAME}.h"
      COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code "${POINTER_CONSTRAINTS_PROTOCOL_XML}" "${POINTER_CONSTRAINTS_GENERATED_NAME}.c"
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    find_path(XKBCOMMON_INCLUDE_DIR NAMES xkbcommon/xkbcommon.h REQUIRED)
    find_library(XKBCOMMON_LIB NAMES xkbcommon REQUIRED)
    target_include_directories(huedra PRIVATE ${XKBCOMMON_INCLUDE_DIR})
    target_link_libraries(huedra PRIVATE ${XKBCOMMON_LIB})

    append_glob_recurse_files(ADDITIONAL_SOURCES 
      ${SHELL_GENERATED_NAME}.c 
      ${DECORATION_GENERATED_NAME}.c 
      ${WARP_GENERATED_NAME}.c 
      ${RELATIVE_POINTER_GENERATED_NAME}.c 
      ${POINTER_CONSTRAINTS_GENERATED_NAME}.c 
      ${PROJECT_SOURCE_DIR}/huedra/platform/wayland/*.cpp 
    )
    target_include_directories(huedra PRIVATE
      ${CMAKE_CURRENT_BINARY_DIR}/wayland
    )

    find_path(SYSTEMD_INCLUDE_DIR
        NAMES systemd/sd-bus.h
    )

    find_library(SYSTEMD_LIBRARY
        NAMES systemd
    )

    if(NOT SYSTEMD_INCLUDE_DIR OR NOT SYSTEMD_LIBRARY)
        message(FATAL_ERROR "Could not find libsystemd!")
    endif()

    target_include_directories(huedra PRIVATE ${SYSTEMD_INCLUDE_DIR})
    target_link_libraries(huedra PRIVATE ${SYSTEMD_LIBRARY})

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