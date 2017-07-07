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

add_library(sample_utils
    SampleUtils.cpp
    SampleUtils.h
)
target_link_libraries(sample_utils utils nxt_wire)
NXTInternalTarget("examples" sample_utils)

function(add_nxt_sample target sources)
    add_executable(${target} ${sources})
    target_link_libraries(${target} sample_utils)
    target_include_directories(${target} PRIVATE ${GLM_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    NXTInternaltarget("examples" ${target})
endfunction()

add_nxt_sample(CHelloTriangle CHelloTriangle.cpp)
add_nxt_sample(CppHelloTriangle HelloTriangle.cpp)
add_nxt_sample(ComputeBoids ComputeBoids.cpp)
add_nxt_sample(HelloVertices HelloVertices.cpp)
add_nxt_sample(HelloInstancing HelloInstancing.cpp)
add_nxt_sample(HelloIndices HelloIndices.cpp)
add_nxt_sample(HelloUBO HelloUBO.cpp)
add_nxt_sample(HelloCompute HelloCompute.cpp)
add_nxt_sample(RenderToTexture RenderToTexture.cpp)
add_nxt_sample(Animometer Animometer.cpp)
add_nxt_sample(CppHelloDepthStencil HelloDepthStencil.cpp)

add_nxt_sample(glTFViewer glTFViewer/glTFViewer.cpp)
