include(${CMAKE_SOURCE_DIR}/cmake/utils.cmake)

add_custom_target(create_link ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${PROJECT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/${SUBFOLDER}/assets
    COMMENT "Creating symlink for assets folder"
)

function(add_example SUBFOLDER)
    append_glob_recurse_files(SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/${SUBFOLDER}/*.cpp 
    )
    
    if(NOT SOURCES)
    message(WARNING "No .cpp files defined in ${CMAKE_CURRENT_SOURCE_DIR}/${SUBFOLDER}")
    return()
    endif()
    
    add_executable(${SUBFOLDER} ${SOURCES})
    
    target_include_directories(${SUBFOLDER} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(${SUBFOLDER} PRIVATE huedra)

    add_dependencies(${SUBFOLDER} create_link)

    # Define compiler flags and macros for different build types
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${SUBFOLDER} PRIVATE -DDEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_definitions(${SUBFOLDER} PRIVATE -DNDEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_definitions(${SUBFOLDER} PRIVATE -DNDEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_definitions(${SUBFOLDER} PRIVATE -DDEBUG)
    endif()
endfunction()

# Create executable for all subfolders
file(GLOB DIRS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(DIR IN LISTS DIRS)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}")
        add_example(${DIR})
    endif()
endforeach()