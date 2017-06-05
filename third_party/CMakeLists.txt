# Copyright 2017 The NXT Authors
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

# GLFW, only build the library
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# GoogleTest
set(GTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest)
set(GMOCK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googlemock)
add_library(gtest STATIC ${GTEST_DIR}/src/gtest-all.cc ${GMOCK_DIR}/src/gmock-all.cc)
target_include_directories(gtest SYSTEM PUBLIC ${GTEST_DIR}/include ${GMOCK_DIR}/include)
target_include_directories(gtest PRIVATE ${GTEST_DIR} ${GMOCK_DIR})
find_package(Threads)
target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})

# Glad
add_library(glad STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/glad/glad.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/KHR/khrplatform.h
)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

# ShaderC
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_GOOGLE_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest CACHE STRING "Location of googletest source")
set(SHADERC_GLSLANG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glslang" CACHE STRING "Location of glslang source")
set(SHADERC_SPIRV_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spirv-tools CACHE STRING "Location of spirv-tools source")
# Need to include this for spirv-tools to find it
add_subdirectory(spirv-headers)
add_subdirectory(shaderc)
SetPIC(shaderc)
SetPIC(shaderc_util)

# SPIRV-Cross
add_library(spirv-cross STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/GLSL.std.450.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cfg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cfg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cpp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cpp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cross.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cross.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_glsl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_glsl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_msl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_msl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_hlsl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_hlsl.hpp
)
target_include_directories(spirv-cross PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
SetCXX14(spirv-cross)

# STB, used for stb_image
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stb PARENT_SCOPE)

# glm matrix math library
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm PARENT_SCOPE)
