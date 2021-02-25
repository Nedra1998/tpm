file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(tpm-bin ${SOURCES})
target_compile_features(tpm-bin PUBLIC cxx_std_20)
set_target_properties(tpm-bin PROPERTIES CXX_EXTENSIONS OFF OUTPUT_NAME tpm)

target_link_libraries(tpm-bin PRIVATE tpm::tpm CLI11::CLI11 magic_enum fmt::fmt)
target_include_directories(tpm-bin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(tpm-bin PROPERTIES INTERPROCEDURAL_OPTIMIZATION
                                         ${TPM_IPO})
target_precompile_headers(tpm-bin PRIVATE)

include(EnableExtraCompilerWarnings)
enable_extra_compiler_warnings(tpm-bin)
analyze_target(TARGET tpm-bin FIX TEST TEST_WERROR)
install(TARGETS tpm-bin)
