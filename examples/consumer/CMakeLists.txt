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

set(_rinos_sdk_public_include_dirs "${_rinos_sdk_include}")
set(_rinos_ringpu_default
    "${CMAKE_CURRENT_LIST_DIR}/../../../libs/RinGPU/include")
set(_rinos_rinresource_default
    "${CMAKE_CURRENT_LIST_DIR}/../../../libs/rinresource/include")
if(NOT EXISTS "${_rinos_sdk_include}/ringpu/ringpu.h")
  set(RINOS_RINGPU_INCLUDE_ROOT "${_rinos_ringpu_default}" CACHE PATH
      "Installed RinGPU public include root required by RinOS GPU headers")
  if(NOT EXISTS "${RINOS_RINGPU_INCLUDE_ROOT}/ringpu/ringpu.h")
    message(FATAL_ERROR
      "RinOS-SDK GPU headers require ringpu/ringpu.h. Set "
      "RINOS_RINGPU_INCLUDE_ROOT to the RinGPU include root, or use a "
      "packaged SDK include tree containing ringpu/.")
  endif()
  list(APPEND _rinos_sdk_public_include_dirs "${RINOS_RINGPU_INCLUDE_ROOT}")
endif()
if(NOT EXISTS "${_rinos_sdk_include}/rinresource/loader.h")
  set(RINOS_RINRESOURCE_INCLUDE_ROOT "${_rinos_rinresource_default}" CACHE PATH
      "Installed RinResource public include root required by RinGPU headers")
  if(NOT EXISTS "${RINOS_RINRESOURCE_INCLUDE_ROOT}/rinresource/loader.h")
    message(FATAL_ERROR
      "RinOS-SDK GPU headers require rinresource/loader.h. Set "
      "RINOS_RINRESOURCE_INCLUDE_ROOT to the RinResource include root, or "
      "use a packaged SDK include tree containing rinresource/.")
  endif()
  list(APPEND _rinos_sdk_public_include_dirs
       "${RINOS_RINRESOURCE_INCLUDE_ROOT}")
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
target_include_directories(rinosdk_headers_c PRIVATE
                           ${_rinos_sdk_public_include_dirs})
add_library(rinosdk_headers_cpp OBJECT ${_rinos_sdk_cpp_probes})
target_include_directories(rinosdk_headers_cpp PRIVATE
                           ${_rinos_sdk_public_include_dirs})

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rinosdk_headers_c PRIVATE -Wall -Wextra -Werror)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rinosdk_headers_cpp PRIVATE -Wall -Wextra -Werror)
endif()

foreach(_example IN ITEMS hello process channel file_portal window media explorer)
  add_library("rinosdk_example_${_example}" OBJECT "${_example}.c")
  target_include_directories("rinosdk_example_${_example}" PRIVATE
                             ${_rinos_sdk_public_include_dirs})
endforeach()

# The keyring wire sample uses the target POSIX local-socket headers. Keep the
# host header-contract project portable to Windows hosts; build.py compiles it
# with the RinOS target toolchain on both supported target architectures.
if(UNIX)
  add_library(rinosdk_example_keyring OBJECT keyring.c)
  target_include_directories(rinosdk_example_keyring PRIVATE
                             ${_rinos_sdk_public_include_dirs})
  add_library(rinosdk_example_ime_provider OBJECT ime_provider.c)
  target_include_directories(rinosdk_example_ime_provider PRIVATE
                             ${_rinos_sdk_public_include_dirs})
endif()

add_library(rinosdk_example_cpp OBJECT example_cpp.cpp)
target_include_directories(rinosdk_example_cpp PRIVATE
                           ${_rinos_sdk_public_include_dirs})

enable_testing()
add_executable(rinosdk_cpp_contract_test cpp_api_contract_test.cpp)
target_include_directories(rinosdk_cpp_contract_test PRIVATE
                           ${_rinos_sdk_public_include_dirs})
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rinosdk_cpp_contract_test PRIVATE
                         -Wall -Wextra -Werror)
endif()
add_test(NAME rinosdk-cpp-api-contract
         COMMAND rinosdk_cpp_contract_test)
