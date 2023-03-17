add_executable (example1
    "example1.cpp")
target_link_libraries (example1
    PRIVATE
        htv)

add_executable (example2
    "ProxyModelWithHeaderModels.h"
    "ProxyModelWithHeaderModels.cpp"
    "example2.cpp")
target_include_directories (example2
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries (example2
    PRIVATE
        htv)
