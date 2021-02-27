cmake_minimum_required(VERSION 3.9...3.19)

project(
  TPM
  VERSION 0.2.0
  LANGUAGES CXX)
string(MAKE_C_IDENTIFIER ${PROJECT_NAME} PROJECT_ID)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Require out-of-source builds
include(RequireOutOfSourceBuild)

# Set default build type
set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS
      "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE
      "${default_build_type}"
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

# Enable link time optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT TPM_IPO)

# CCache compiler launcher configuration
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
endif()

# Update git submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  # Update submodules as needed
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(
        FATAL_ERROR
          "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
      )
    endif()
  endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/spdlog/CMakeLists.txt")
  message(
    FATAL_ERROR
      "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again."
  )
endif()

# Configure compile command database
if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
  set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
endif()

# Enable testing
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()

include(Format)
add_formatting(
  FIX
  TEST
  TEST_ARGS
  ${CLANGFORMAT_WERROR}
  EXCLUDE
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/extern")

include(Analyzers)
global_analyze()

find_package(OpenCL)

add_subdirectory(extern/CLI11 EXCLUDE_FROM_ALL)
add_subdirectory(extern/Catch2 EXCLUDE_FROM_ALL)
add_subdirectory(extern/fmt EXCLUDE_FROM_ALL)
add_subdirectory(extern/glm EXCLUDE_FROM_ALL)
add_subdirectory(extern/magic_enum EXCLUDE_FROM_ALL)
add_subdirectory(extern/spdlog EXCLUDE_FROM_ALL)

add_subdirectory(apps)
add_subdirectory(src)

# option(TPM_BUILD_TESTING "Build testing targets for TPM" OFF)
# if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR TPM_BUILD_TESTING) AND
# BUILD_TESTING) add_subdirectory(extern/Catch2) list(APPEND CMAKE_MODULE_PATH
# "${PROJECT_SOURCE_DIR}/extern/Catch2/extras") add_subdirectory(tests) endif()
