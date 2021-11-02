file (GLOB CHILDREN RELATIVE ${CMAKE_CURRENT_LIST_DIR} "*")
foreach (CHILD ${CHILDREN})
    if (IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${CHILD})
        message (STATUS "Adding plugin: ${CHILD}")
        add_subdirectory (${CHILD})
    endif ()
endforeach ()

