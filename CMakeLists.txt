set(PROJECT_NAME ReadPl4)
set(VERSION_VAL 1.0.0)
set(LANGUAGES_VAL CXX)

message("")
message("> ----------------------------------------------------------------")
message("> [Project] ${PROJECT_NAME}")
message("> [Version] ${VERSION_VAL}")
message("> [Languages] ${LANGUAGES_VAL}")
message("> ----------------------------------------------------------------")
message("/")

# cmake
cmake_minimum_required(VERSION 3.20)

# project
message("project")
project(${PROJECT_NAME}
VERSION ${VERSION_VAL}
LANGUAGES ${LANGUAGES_VAL}
)

# Global Variables
set(PROJECT_VERSION "${PROJECT_NAME} ${VERSION_VAL}")
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")

# include directory
include_directories(${CMAKE_SOURCE_DIR}/dist/include)

# subdirectory
add_subdirectory(${CMAKE_SOURCE_DIR}/src)