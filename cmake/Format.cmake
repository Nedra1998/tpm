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

if(__add_formatting)
  return()
endif()
set(__add_formatting YES)

if(NOT CLANGFORMAT_FOUND)
  find_package(clangformat)
endif()

function(add_formatting)
  set(options FIX TEST)
  set(oneValueArgs TARGET NAME)
  set(multiValueArgs SOURCES ARGS TEST_ARGS EXCLUDE)
  cmake_parse_arguments(FMT "${options}" "${oneValueArgs}" "${multiValueArgs}"
                        ${ARGN})

  if(NOT FMT_NAME AND FMT_TARGET)
    set(FMT_NAME "${FMT_TARGET}-fmt")
  elseif(NOT FMT_NAME AND NOT FMT_TARGET)
    set(FMT_NAME "format")
  endif()

  if(FMT_TARGET AND TARGET ${FMT_TARGET})
    get_target_property(target_sources ${FMT_TARGET} SOURCES)
    foreach(src ${target_sources})
      get_source_file_property(source_lang ${src} LANGUAGE)
      get_source_file_property(source_loc ${src} LOCATION)
      if("${source_lang}" MATCHES "C|CXX")
        list(APPEND FMT_SOURCES "${source_loc}")
      endif()
    endforeach()
  endif()

  if(NOT FMT_SOURCES)
    file(GLOB_RECURSE cpp_sources CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    list(APPEND FMT_SOURCES ${cpp_sources})
    file(GLOB_RECURSE hpp_sources CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
    list(APPEND FMT_SOURCES ${hpp_sources})
  endif()

  foreach(exc ${FMT_EXCLUDE})
    list(FILTER FMT_SOURCES EXCLUDE REGEX ${exc})
  endforeach()

  if(FMT_SOURCES AND CLANGFORMAT_FOUND)
    add_custom_target(
      ${FMT_NAME}
      COMMAND ${CLANGFORMAT_EXECUTABLE} ${CLANGFORMAT_DRYRUN} ${ARGS}
              ${FMT_SOURCES}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "${FMT_NAME}: Running clang-format checks"
      DEPENDS ${FMT_SOURCES})

    if(FMT_FIX)
      add_custom_target(
        ${FMT_NAME}-fix
        COMMAND ${CLANGFORMAT_EXECUTABLE} ${CLANGFORMAT_INPLACE} ${ARGS}
                ${FMT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "${FMT_NAME}-fix: Applying clang-format fixes"
        DEPENDS ${FMT_SOURCES})
    endif()

    if(FMT_TEST)
      add_test(NAME ${FMT_NAME}
               COMMAND ${CLANGFORMAT_EXECUTABLE} ${CLANGFORMAT_DRYRUN} ${ARGS}
                       ${FMT_TEST_ARGS} ${FMT_SOURCES})
    endif()
  endif()

endfunction()
