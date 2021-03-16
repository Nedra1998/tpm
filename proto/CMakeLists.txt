file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")

set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()

string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" BUILD_SOURCES ${SOURCES})
string(REGEX REPLACE "[.]proto$" ".pb.cc" PROTO_SRCS ${BUILD_SOURCES})
string(REGEX REPLACE "[.]proto$" ".pb.h" PROTO_HDRS ${BUILD_SOURCES})
string(REGEX REPLACE "[.]proto$" ".grpc.pb.cc" GRPC_SRCS ${BUILD_SOURCES})
string(REGEX REPLACE "[.]proto$" ".grpc.pb.h" GRPC_HDRS ${BUILD_SOURCES})

add_custom_command(OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
    --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
    -I "${CMAKE_CURRENT_SOURCE_DIR}"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    --experimental_allow_proto3_optional
    "${SOURCES}"
  DEPENDS "${SOURCES}")

add_library(tpm-grpc ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS})
add_library(tpm::grpc ALIAS tpm-grpc)
target_link_libraries(tpm-grpc PUBLIC ${_REFLECTION} ${_GRPC_GROPCPP} ${_PROTOBUF_LIBPROTOBUF})
target_include_directories(tpm-grpc PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
