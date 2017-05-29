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

list(APPEND UTILS_SOURCES
    Utils.h
    Utils.cpp
    BackendBinding.h
)

if (APPLE)
    list(APPEND UTILS_SOURCES
        MetalBinding.mm
    )
endif()

add_library(utils STATIC ${UTILS_SOURCES})
target_link_libraries(utils nxt_backend nxt_wire shaderc nxtcpp nxt)
SetCXX14(utils)

add_executable(CHelloTriangle HelloTriangle.c)
target_link_libraries(CHelloTriangle utils)

add_executable(CppHelloTriangle HelloTriangle.cpp)
target_link_libraries(CppHelloTriangle utils)
SetCXX14(CppHelloTriangle)

add_executable(ComputeBoids ComputeBoids.cpp)
target_link_libraries(ComputeBoids utils)
target_include_directories(ComputeBoids PUBLIC ../ ${GLM_INCLUDE_DIR})
SetCXX14(ComputeBoids)

add_executable(HelloVertices HelloVertices.cpp)
target_link_libraries(HelloVertices utils)
SetCXX14(HelloVertices)

add_executable(HelloInstancing HelloInstancing.cpp)
target_link_libraries(HelloInstancing utils)
SetCXX14(HelloInstancing)

add_executable(HelloIndices HelloIndices.cpp)
target_link_libraries(HelloIndices utils)
SetCXX14(HelloIndices)

add_executable(HelloUBO HelloUBO.cpp)
target_link_libraries(HelloUBO utils)
SetCXX14(HelloUBO)

add_executable(HelloCompute HelloCompute.cpp)
target_link_libraries(HelloCompute utils)
SetCXX14(HelloCompute)

add_executable(RenderToTexture RenderToTexture.cpp)
target_link_libraries(RenderToTexture utils)
SetCXX14(RenderToTexture)

add_executable(Animometer Animometer.cpp)
target_link_libraries(Animometer utils)
SetCXX14(Animometer)

add_executable(SpirvTest SpirvTest.cpp)
target_link_libraries(SpirvTest shaderc spirv-cross nxtcpp)
SetCXX14(SpirvTest)

add_subdirectory(glTFViewer)
