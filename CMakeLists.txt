cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(
  TPM
  VERSION 0.3.0
  LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(GenerateVersionHeader)
include(RequireOutOfSourceBuild)
include(SetDefaultBuildType)
include(EnableExtraCompilerWarnings)

set_default_build_type("Debug")

if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
  set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
endif()

if(NOT HIPSYCL_TARGETS)
  set(HIPSYCL_TARGETS "omp")
endif()
find_package(hipSYCL CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "7.1.3")
FetchContent_MakeAvailable(fmt)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY "https://github.com/gabime/spdlog"
  GIT_TAG "v1.8.5")
FetchContent_MakeAvailable(spdlog)
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY "https://github.com/jarro2783/cxxopts")
FetchContent_MakeAvailable(cxxopts)
FetchContent_Declare(
  pugixml
  GIT_REPOSITORY "https://github.com/zeux/pugixml")
FetchContent_MakeAvailable(pugixml)

add_subdirectory(src)
