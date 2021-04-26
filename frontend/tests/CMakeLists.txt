project ("uh-tests"
	VERSION 0.0.1
	LANGUAGES CXX)
	
add_executable (uh-tests
    "src/test_Reference.cpp"
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
		RUNTIME_OUTPUT_DIRECTORY ${UH_BUILD_BINDIR})
