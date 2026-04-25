message("/src")

# src
set(SRC_FILES
${CMAKE_CURRENT_LIST_DIR}/PL4.h
)

# test
set(TEST_FILES
${SRC_FILES}
${CMAKE_CURRENT_LIST_DIR}/Testing/test_PL4.cpp
)

add_executable(readPl4 ${TEST_FILES})

set_target_properties(readPl4 PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/bin
)

target_include_directories(readPl4 
PUBLIC ${CMAKE_CURRENT_LIST_DIR})