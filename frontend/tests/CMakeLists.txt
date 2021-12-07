project ("uh-tests"
	VERSION ${UH_VERSION}
	LANGUAGES CXX)
	
add_executable (uh-tests
    "src/test_HashMap.cpp"
    "src/test_Reference.cpp"
    "src/test_SmallLinearMap.cpp"
    "src/test_SmallString.cpp"
    "src/test_SmallVector.cpp"
    "src/test_String.cpp"
    "src/test_Vector.cpp"
	"src/main.cpp")
target_link_libraries (uh-tests
	PRIVATE uh
	PRIVATE gmock)
set_target_properties (uh-tests
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${UH_BUILD_BINDIR}
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${UH_BUILD_BINDIR}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${UH_BUILD_BINDIR})
