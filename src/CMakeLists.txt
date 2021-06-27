file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
generate_version_header(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp")

add_executable(tpm ${SOURCES})
target_compile_features(tpm PUBLIC cxx_std_17)
target_include_directories(tpm PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(tpm PRIVATE fmt spdlog cxxopts)

option(ENABLE_PROF "Enable profiling" OFF)
if(ENABLE_PROF)
  target_compile_definitions(tpm PRIVATE -DUSE_PL=1)
endif()

add_sycl_to_target(TARGET tpm)
enable_extra_compiler_warnings(tpm)
