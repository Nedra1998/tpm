# Copyright (c) 2020 Rasmussen, Arden
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

if(__gernerate_version_header)
  return()
endif()
set(__gernerate_version_header YES)

find_package(Git)
set(_version_header_template
    "${CMAKE_CURRENT_LIST_DIR}/GenerateVersionHeader.hpp.in")

function(generate_version_header)
  set(OPTIONS)
  set(SINGLE_VALUE_KEYWORDS OUTPUT_DIR RELEASE)
  set(MULTI_VALUE_KEYWORDS)
  cmake_parse_arguments(ver "${OPTIONS}" "${SINGLE_VALUE_KEYWORDS}"
                        "${MULTI_VALUE_KEYWORDS}" ${ARGN})

  string(MAKE_C_IDENTIFIER ${PROJECT_NAME} _id)
  string(TOUPPER ${_id} PROJECT_UPPER)
  string(TOLOWER ${_id} PROJECT_LOWER)

  if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE PROJECT_VERSION_BUILD
      RESULT_VARIABLE git_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()

  if(NOT ver_OUTPUT_DIR)
    set(ver_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  if(ver_RELEASE)
    set(PROJECT_VERSION_RELEASE "${ver_RELEASE}")
  endif()

  set(PROJECT_VERSION_CORE
      "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
  )
  set(PROJECT_SEMVER "${PROJECT_VERSION_CORE}")
  if(PROJECT_VERSION_RELEASE)
    set(PROJECT_SEMVER "${PROJECT_SEMVER}-${PROJECT_VERSION_RELEASE}")
  endif()
  if(PROJECT_VERSION_BUILD)
    set(PROJECT_SEMVER "${PROJECT_SEMVER}+${PROJECT_VERSION_BUILD}")
  endif()
  if(NOT EXISTS ${_version_header_template})
    message(
      FATAL_ERROR
        "Missing template file ${_version_header_template} - should be alongside GenerateVersionHeader.cmake"
    )
  endif()
  configure_file("${_version_header_template}" "${ver_OUTPUT_DIR}/version.hpp")

endfunction()
