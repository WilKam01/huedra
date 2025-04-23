include(FetchContent)

# Shader-Slang
set(SLANG_VERSION "2025.7")

if(WIN32)
    set(SLANG_OS "windows")
    set(SLANG_ARCH "x86_64")
elseif(APPLE)
    set(SLANG_OS "macos")
    set(SLANG_ARCH "aarch64")
endif()

FetchContent_Declare(
    slang_zip
    URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-${SLANG_OS}-${SLANG_ARCH}.zip 
    QUIET
)
FetchContent_MakeAvailable(slang_zip)

add_library(slang UNKNOWN IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(slang PROPERTIES 
        IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/bin/slang.dll
        IMPORTED_IMPLIB ${slang_zip_SOURCE_DIR}/lib/slang.lib
        INTERFACE_INCLUDE_DIRECTORIES ${slang_zip_SOURCE_DIR}/include)
elseif(APPLE)
    set_target_properties(slang PROPERTIES 
        IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/lib/libslang.dylib
        INTERFACE_INCLUDE_DIRECTORIES ${slang_zip_SOURCE_DIR}/include)
endif()