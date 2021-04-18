project ("uh-tests"
	VERSION 0.0.1
	LANGUAGES CXX)
	
add_executable (uh-tests
	"src/test_reference.cpp"
	"src/main.cpp")
target_link_libraries (uh-tests
	PRIVATE uh
	PRIVATE gmock)
set_target_properties (uh-tests
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${UH_BUILD_BINDIR})
