file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

include(GenerateVersionHeader)
generate_version_header(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/tpm")
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/include/tpm/version.hpp")

include(WriteCompilerDetectionHeader)
write_compiler_detection_header(
  FILE "${CMAKE_CURRENT_BINARY_DIR}/include/tpm/compiler.hpp" PREFIX
  ${PROJECT_ID}
  OUTPUT_FILES_VAR COMPILER_SUPPORT_FILES
  OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/tpm/"
  COMPILERS AppleClang Clang GNU Intel MSVC
  FEATURES cxx_constexpr
           cxx_deleted_functions
           cxx_extern_templates
           cxx_final
           cxx_noexcept
           cxx_override
           cxx_alignas
           cxx_alignof
           cxx_nullptr
           cxx_static_assert
           cxx_attribute_deprecated
           cxx_thread_local)
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/include/tpm/compiler.hpp"
     ${COMPILER_SUPPORT_FILES})

add_library(tpm ${SOURCES})
add_library(tpm::tpm ALIAS tpm)
target_compile_features(tpm PUBLIC cxx_std_20)
set_target_properties(tpm PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(
  tpm
  PUBLIC spdlog glm
  PRIVATE magic_enum fmt::fmt)
target_include_directories(
  tpm
  PUBLIC ${PROJECT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_BINARY_DIR}/include/
  PRIVATE ${PROJECT_SOURCE_DIR}/include/tpm
          ${CMAKE_CURRENT_BINARY_DIR}/include/tpm)

if(OpenCL_FOUND)
  target_link_libraries(tpm PRIVATE OpenCL::OpenCL)
endif()

set_target_properties(tpm PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${TPM_IPO})
target_precompile_headers(tpm PRIVATE <string>)

include(EnableExtraCompilerWarnings)
enable_extra_compiler_warnings(tpm)
analyze_target(TARGET tpm FIX TEST TEST_WERROR)
install(TARGETS tpm)
