cmake_minimum_required(VERSION 3.14...4.3.4)

project(ai-utils-testsuite
        LANGUAGES CXX
        DESCRIPTION "Testsuite for git submodule ai-utils.")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Begin of gitache configuration.
set(GITACHE_PACKAGES libcwd)
include(cmake/aicxx/cmake/StableGitache)
# End of gitache configuration.

# This project uses aicxx modules.
include(cmake/aicxx/Project NO_POLICY_SCOPE)

#Exit if someone tries to contaminates the source directory with an in-source build.
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Please do out-of-source builds.\nCleanup: \"rm -rf CMake CMakeCache.txt CMakeFiles/\"")
endif()

include(AICxxSubmodules)

# Find Google Test package
find_package(GTest)

add_subdirectory(microbench)
add_subdirectory(src)

# We use utils/to_string.h
add_subdirectory(enchantum)
