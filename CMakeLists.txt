cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)
# --- Project ---
project(TPM VERSION 0.1.0 LANGUAGES C)

if("${CMAKE_GENERATOR}" MATCHES "Makefiles" AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

if("${CMAKE_BUILD_TYPE}" MATCHES "Debug|RelWithDebInfo")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  if (NOT EXISTS "${PROJECT_SOURCE_DIR}/compile_commands.json")
    execute_process(COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      ${PROJECT_BINARY_DIR}/compile_commands.json
      ${PROJECT_SOURCE_DIR}/compile_commands.json)
  endif()
endif()

# --- Dependencies ---

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)
include(WriteCompilerDetectionHeader)
include(cotire)

# --- Sub Directories ---

add_subdirectory(src)
