# Copyright (c) 2021 Rasmussen, Arden
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

if(__analyze_target)
  return()
endif()
set(__analyze_target YES)

if(NOT CPPCHECK_FOUND)
  find_package(cppcheck)
endif()

if(NOT CLANGTIDY_FOUND)
  find_package(clangtidy)
endif()

function(global_analyze)
  add_custom_target(analyze COMMAND;)
  add_custom_target(analyze-fix COMMAND;)
endfunction()

function(analyze_target)
  set(options
      CPPCHECK
      CLANGTIDY
      TEST
      FIX
      WERROR
      QUIET
      TEST_WERROR
      TEST_QUIET)
  set(oneValueArgs TARGET NAME)
  set(multiValueArgs SOURCES EXCLUDE)
  cmake_parse_arguments(ANLZ "${options}" "${oneValueArgs}" "${multiValueArgs}"
                        ${ARGN})

  if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(_compile_database "${PROJECT_BINARY_DIR}/compile_commands.json")
  endif()

  if(NOT ANLZ_NAME AND ANLZ_TARGET)
    set(ANLZ_NAME "${ANLZ_TARGET}-analyze")
  elseif(NOT ANLZ_NAME AND NOT ANLZ_TARGET)
    set(ANLZ_NAME "analyze")
  endif()

  if(ANLZ_TARGET AND TARGET ${ANLZ_TARGET})
    get_target_property(target_sources ${ANLZ_TARGET} SOURCES)
    foreach(src ${target_sources})
      get_source_file_property(source_lang ${src} LANGUAGE)
      get_source_file_property(source_loc ${src} LOCATION)
      if("${source_lang}" MATCHES "C|CXX")
        list(APPEND ANLZ_SOURCES "${source_loc}")
      endif()
    endforeach()
  endif()

  if(NOT ANLZ_SOURCES)
    file(GLOB_RECURSE cpp_sources CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    list(APPEND ANLZ_SOURCES ${cpp_sources})
    file(GLOB_RECURSE hpp_sources CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
    list(APPEND ANLZ_SOURCES ${hpp_sources})
  endif()

  foreach(exc ${ANLZ_EXCLUDE})
    list(FILTER ANLZ_SOURCES EXCLUDE REGEX ${exc})
  endforeach()

  if(CPPCHECK_FOUND)
    set(cppcheck_cmd "${CPPCHECK_EXECUTABLE}" "${CPPCHECK_ALL}")
    set(cppcheck_target_cmd "")
    set(cppcheck_test_cmd "")
    if(_compile_database)
      list(APPEND cppcheck_cmd "${CPPCHECK_DATABASE}${_compile_database}")
    endif()
    if(ANLZ_QUIET)
      list(APPEND cppcheck_target_cmd "${CPPCHECK_QUIET}")
    endif()
    if(ANLZ_WERROR)
      list(APPEND cppcheck_target_cmd "${CPPCHECK_WERROR}")
    endif()
    if(ANLZ_TEST_QUIET)
      list(APPEND cppcheck_test_cmd "${CPPCHECK_QUIET}")
    endif()
    if(ANLZ_TEST_WERROR)
      list(APPEND cppcheck_test_cmd "${CPPCHECK_WERROR}")
    endif()
    list(APPEND cppcheck_cmd "-i${CMAKE_CURRENT_BINARY_DIR}")
    if(CPM_SOURCE_CACHE)
      list(APPEND cppcheck_cmd "-i${CPM_SOURCE_CACHE}")
    endif()
  endif()

  if(CLANGTIDY_FOUND)
    set(clangtidy_cmd "${CLANGTIDY_EXECUTABLE}")
    set(cppcheck_target_cmd "")
    set(cppcheck_test_cmd "")
    if(_compile_database)
      list(APPEND clangtidy_cmd "${CLANGTIDY_DATABASE}" "${_compile_database}")
    endif()
    if(ANLZ_QUIET)
      list(APPEND clangtidy_target_cmd "${CLANGTIDY_QUIET}")
    endif()
    if(ANLZ_WERROR)
      list(APPEND clangtidy_target_cmd "${CLANGTIDY_WERROR}")
    endif()
    if(ANLZ_TEST_QUIET)
      list(APPEND clangtidy_test_cmd "${CLANGTIDY_QUIET}")
    endif()
    if(ANLZ_TEST_WERROR)
      list(APPEND clangtidy_test_cmd "${CLANGTIDY_WERROR}")
    endif()
  endif()

  if(ANLZ_SOURCES)
    set(_targets)
    set(_fix_targets)
    if(cppcheck_cmd)
      add_custom_target(
        ${ANLZ_NAME}-cppcheck
        COMMAND ${cppcheck_cmd} ${cppcheck_target_cmd} ${ANLZ_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "${ANLZ_NAME}-cppcheck: Running cppcheck analizer"
        DEPENDS ${ANLZ_SOURCES})
      list(APPEND _targets ${ANLZ_NAME}-cppcheck)

      if(ANLZ_TEST)
        add_test(NAME ${ANLZ_NAME}-cppcheck
                 COMMAND ${cppcheck_cmd} ${cppcheck_test_cmd} ${ANLZ_SOURCES})
      endif()
    endif()
    if(clangtidy_cmd)
      add_custom_target(
        ${ANLZ_NAME}-clangtidy
        COMMAND ${clangtidy_cmd} ${clangtidy_target_cmd} ${ANLZ_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "${ANLZ_NAME}-clangtidy: Running clang-tidy analizer"
        DEPENDS ${ANLZ_SOURCES})
      list(APPEND _targets ${ANLZ_NAME}-clangtidy)
      if(ANLZ_FIX)
        add_custom_target(
          ${ANLZ_NAME}-clangtidy-fix
          COMMAND ${clangtidy_cmd} ${CLANGTIDY_FIX} ${clangtidy_target_cmd}
                  ${ANLZ_SOURCES}
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMENT "${ANLZ_NAME}-clangtidy: Applying clang-tidy analizer fixes"
          DEPENDS ${ANLZ_SOURCES})
        list(APPEND _fix_targets ${ANLZ_NAME}-clangtidy-fix)
      endif()
      if(ANLZ_TEST)
        add_test(NAME ${ANLZ_NAME}-clangtidy
                 COMMAND ${clangtidy_cmd} ${clangtidy_test_cmd} ${ANLZ_SOURCES})
      endif()
    endif()

    add_custom_target(
      ${ANLZ_NAME}
      COMMAND ;
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${_targets})
    if(TARGET analyze)
      add_dependencies(analyze ${ANLZ_NAME})
    endif()

    if(ANLZ_FIX)
      add_custom_target(
        ${ANLZ_NAME}-fix
        COMMAND ;
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_fix_targets})
      if(TARGET analyze-fix)
        add_dependencies(analyze-fix ${ANLZ_NAME}-fix)
      endif()
    endif()
  endif()

endfunction()
