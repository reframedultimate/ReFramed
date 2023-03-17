project ("ReFramed-tests"
    VERSION ${REFRAMED_VERSION}
    LANGUAGES CXX)

add_executable (ReFramed-tests
    "src/test_Deque.cpp"
    "src/test_FreeList.cpp"
    "src/test_HashMap.cpp"
    "src/test_MotionLabels.cpp"
    "src/test_Queue.cpp"
    "src/test_Reference.cpp"
    "src/test_SmallLinearMap.cpp"
    "src/test_SmallString.cpp"
    "src/test_SmallVector.cpp"
    "src/test_String.cpp"
    "src/test_Vector.cpp"
    "src/main.cpp")
target_link_libraries (ReFramed-tests
    PRIVATE rfcommon
    PRIVATE gmock)
set_target_properties (ReFramed-tests
    PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${REFRAMED_BUILD_BINDIR})

add_custom_command (TARGET ReFramed-tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${REFRAMED_BUILD_DATADIR}/tests"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/data/hash40_1.csv" "${REFRAMED_BUILD_DATADIR}/tests"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/data/hash40_2.csv" "${REFRAMED_BUILD_DATADIR}/tests"
    COMMENT "Copying test data files to ${REFRAMED_BUILD_DATADIR}/tests"
    VERBATIM)
    
