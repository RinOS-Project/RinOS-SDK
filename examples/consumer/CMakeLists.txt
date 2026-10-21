cmake_minimum_required(VERSION 3.20)
project(RinOSSDKConsumerExamples LANGUAGES C CXX)

set(_rinos_sdk_default "${CMAKE_CURRENT_LIST_DIR}/../..")
get_filename_component(_rinos_sdk_default "${_rinos_sdk_default}" ABSOLUTE)
set(RINOS_SDK_ROOT "${_rinos_sdk_default}" CACHE PATH
    "Root of a RinOS-SDK checkout")
get_filename_component(RINOS_SDK_ROOT "${RINOS_SDK_ROOT}" ABSOLUTE)
set(_rinos_sdk_include "${RINOS_SDK_ROOT}/include")
if(NOT EXISTS "${_rinos_sdk_include}/rin/sdk.h")
  message(FATAL_ERROR "RINOS_SDK_ROOT does not contain include/rin/sdk.h")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE _rinos_sdk_c_headers CONFIGURE_DEPENDS
     LIST_DIRECTORIES FALSE "${_rinos_sdk_include}/rin/*.h")
file(GLOB_RECURSE _rinos_sdk_cpp_headers CONFIGURE_DEPENDS
     LIST_DIRECTORIES FALSE
     "${_rinos_sdk_include}/rin/*.h"
     "${_rinos_sdk_include}/rin/*.hpp")

set(_rinos_sdk_c_probes)
foreach(_header IN LISTS _rinos_sdk_c_headers)
  file(RELATIVE_PATH _relative "${_rinos_sdk_include}" "${_header}")
  string(SHA256 _digest "${_relative}")
  string(SUBSTRING "${_digest}" 0 16 _probe_id)
  set(_probe "${CMAKE_CURRENT_BINARY_DIR}/generated/c/${_probe_id}.c")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/c")
  file(WRITE "${_probe}" "#include <${_relative}>\n")
  list(APPEND _rinos_sdk_c_probes "${_probe}")
endforeach()

set(_rinos_sdk_cpp_probes)
foreach(_header IN LISTS _rinos_sdk_cpp_headers)
  file(RELATIVE_PATH _relative "${_rinos_sdk_include}" "${_header}")
  string(SHA256 _digest "${_relative}")
  string(SUBSTRING "${_digest}" 0 16 _probe_id)
  set(_probe "${CMAKE_CURRENT_BINARY_DIR}/generated/cpp/${_probe_id}.cpp")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/cpp")
  file(WRITE "${_probe}" "#include <${_relative}>\n")
  list(APPEND _rinos_sdk_cpp_probes "${_probe}")
endforeach()

add_library(rinosdk_headers_c OBJECT ${_rinos_sdk_c_probes})
target_include_directories(rinosdk_headers_c PRIVATE "${_rinos_sdk_include}")
add_library(rinosdk_headers_cpp OBJECT ${_rinos_sdk_cpp_probes})
target_include_directories(rinosdk_headers_cpp PRIVATE "${_rinos_sdk_include}")

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rinosdk_headers_c PRIVATE -Wall -Wextra -Werror)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rinosdk_headers_cpp PRIVATE -Wall -Wextra -Werror)
endif()

foreach(_example IN ITEMS hello process channel file_portal window)
  add_library("rinosdk_example_${_example}" OBJECT "${_example}.c")
  target_include_directories("rinosdk_example_${_example}" PRIVATE
                             "${_rinos_sdk_include}")
endforeach()

add_library(rinosdk_example_cpp OBJECT example_cpp.cpp)
target_include_directories(rinosdk_example_cpp PRIVATE "${_rinos_sdk_include}")
