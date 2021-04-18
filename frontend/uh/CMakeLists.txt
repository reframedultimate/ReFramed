include (TestVisibilityMacros)

project ("uh"
	VERSION 0.0.1
	LANGUAGES CXX)
	
test_visibility_macros (
	UH_API_EXPORT 
	UH_API_IMPORT 
	UH_API_LOCAL)
	
configure_file ("templates/config.hpp.in"
                "include/uh/config.hpp")

add_library (uh SHARED
	"src/ActiveRecording.cpp"
	"src/DataPoint.cpp"
	"src/DataSet.cpp"
	"src/DecisionGraph.cpp"
	"src/FighterIDMapping.cpp"
	"src/FighterStatusMapping.cpp"
	"src/HitStatusMapping.cpp"
	"src/MappingInfo.cpp"
	"src/PlayerState.cpp"
	"src/Recording.cpp"
	"src/RefCounted.cpp"
	"src/SavedRecording.cpp"
	"src/SetFormat.cpp"
	"src/StageIDMapping.cpp"
    "src/platform/tcp_socket.c")
target_include_directories (uh
	PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
	PUBLIC $<INSTALL_INTERFACE:include>)
target_compile_definitions (uh
	PRIVATE UH_BUILDING)
target_compile_options (uh
	PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
	PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>)
target_compile_features (uh
	PUBLIC cxx_std_17)
set_target_properties (uh
	PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${UH_BUILD_LIBDIR}
		RUNTIME_OUTPUT_DIRECTORY ${UH_BUILD_BINDIR})

install (
	TARGETS uh
	EXPORT UHConfig
	LIBRARY DESTINATION ${UH_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${UH_INSTALL_BINDIR})
install (
	DIRECTORY "include/uh"
	DESTINATION ${UH_INSTALL_INCLUDEDIR})
install (
	DIRECTORY "${PROJECT_BINARY_DIR}/include/uh"
	DESTINATION ${UH_INSTALL_INCLUDEDIR})
install (
	EXPORT UHConfig
	DESTINATION ${UH_INSTALL_CMAKEDIR})
