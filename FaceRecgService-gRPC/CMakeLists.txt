# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.11.1)

project(FaceRecgService C CXX)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

find_package(Threads REQUIRED)

set(GRPC_FETCHCONTENT true)
if(GRPC_FETCHCONTENT)
  # Another way is to use CMake's FetchContent module to clone gRPC at
  # configure time. This makes gRPC's source code available to your project,
  # similar to a git submodule.
  message(STATUS "Using gRPC via add_subdirectory (FetchContent).")
  include(FetchContent)
  FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    # when using gRPC, you will actually set this to an existing tag, such as
    # v1.25.0, v1.26.0 etc..
    # For the purpose of testing, we override the tag used to the commit
    # that's currently under test.
    GIT_TAG        v1.28.1)
  FetchContent_MakeAvailable(grpc)

  # Since FetchContent uses add_subdirectory under the hood, we can use
  # the grpc targets directly from this build.
  set(_PROTOBUF_LIBPROTOBUF libprotobuf)
  set(_REFLECTION grpc++_reflection)
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
  set(_GRPC_GRPCPP grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
  endif()
else()
  message(FATAL_ERROR "No gRPC found!!!")
endif()

# Proto file
get_filename_component(fr_proto "protos/FaceRecg.proto" ABSOLUTE)
get_filename_component(fr_proto_path "${fr_proto}" PATH)

# Generated sources
set(fr_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/FaceRecg.pb.cc")
set(fr_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/FaceRecg.pb.h")
set(fr_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/FaceRecg.grpc.pb.cc")
set(fr_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/FaceRecg.grpc.pb.h")
add_custom_command(
      OUTPUT "${fr_proto_srcs}" "${fr_proto_hdrs}" "${fr_grpc_srcs}" "${fr_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${fr_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${fr_proto}"
      DEPENDS "${fr_proto}")

# Include generated *.pb.h files.
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
# Include opencv hdrs.
include_directories("libs/opencv/include")
# Include alg hdrs of face recognizing.
include_directories("inc")
# Extra libs
file(GLOB_RECURSE EXT_LIBS 
      ${CMAKE_CURRENT_SOURCE_DIR}/libs/opencv/lib/libopencv_imgproc.so
      ${CMAKE_CURRENT_SOURCE_DIR}/libs/opencv/lib/libopencv_imgcodecs.so
      ${CMAKE_CURRENT_SOURCE_DIR}/libs/opencv/lib/libopencv_highgui.so
      ${CMAKE_CURRENT_SOURCE_DIR}/libs/opencv/lib/libopencv_core.so
      ${CMAKE_CURRENT_SOURCE_DIR}/libs/shared/libface_recognize.so
)

# Targets greeter_[async_](client|server)
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
# message(STATUS "SRC  ${SRC}/src")
foreach(_target greeter_client greeter_server)
  add_executable(${_target} "${SRC}/${_target}.cc"
    ${fr_proto_srcs}
    ${fr_grpc_srcs})

  if(_target MATCHES ".*server.*")
    # message(STATUS "SER: ${_target}")
    target_link_libraries(${_target}
      ${_REFLECTION}
      ${_GRPC_GRPCPP}
      ${_PROTOBUF_LIBPROTOBUF}
      ${EXT_LIBS})
  else()
    # message(STATUS "CLI: ${_target}")
    target_link_libraries(${_target}
      ${_REFLECTION}
      ${_GRPC_GRPCPP}
      ${_PROTOBUF_LIBPROTOBUF})
  endif()

endforeach()