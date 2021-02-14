cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

# --- Sources ---

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

write_compiler_detection_header(
  FILE "${CMAKE_CURRENT_BINARY_DIR}/compiler.h"
          PREFIX TPM
          COMPILERS GNU Clang AppleClang Intel MSVC
          FEATURES c_function_prototypes c_restrict c_static_assert c_variadic_macros c_std_11
)
list(APPEND HEADERS "${CMAKE_CURRENT_BINARY_DIR}/compiler.h")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h")
list(APPEND HEADERS "${CMAKE_CURRENT_BINARY_DIR}/version.h")

# --- Target ---

add_executable(tpm ${SOURCES} ${HEADERS})
target_include_directories(tpm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(tpm PRIVATE m)

cotire(tpm)
