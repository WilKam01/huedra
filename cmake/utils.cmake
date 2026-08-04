function(append_glob_recurse_files INPUT_LIST)
    file(GLOB_RECURSE TEMP_LIST ${ARGN})
    set(CURRENT_LIST ${${INPUT_LIST}})
    list(APPEND CURRENT_LIST ${TEMP_LIST})
    set(${INPUT_LIST} ${CURRENT_LIST} PARENT_SCOPE) # Return new list
endfunction()