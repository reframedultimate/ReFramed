project ("ReFramed-tests"
    VERSION ${REFRAMED_VERSION}
    LANGUAGES CXX)

add_executable (ReFramed-tests
    "src/test_HashMap.cpp"
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
        RUNTIME_OUTPUT_DIRECTORY ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${REFRAMED_BUILD_BINDIR})
