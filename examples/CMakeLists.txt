# Copyright 2017 The Dawn Authors
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
target_link_libraries(sample_utils utils dawn_wire)
NXTInternalTarget("examples" sample_utils)

function(add_dawn_sample target sources)
    add_executable(${target} ${sources})
    target_link_libraries(${target} sample_utils)
    target_include_directories(${target} SYSTEM PRIVATE ${GLM_INCLUDE_DIR})
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    NXTInternalTarget("examples" ${target})

    # Suppress some warnings in our sample dependencies
    if (MSVC)
        # nonstandard extension used: nameless struct/union -- for GLM
        set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS "/wd4201")
        # declaration hides global declaration -- for GLM
        set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS "/wd4459")
        # = conversion possible loss of data -- for STB image
        set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS "/wd4244")
        # declaration hides previous declaration -- for STB image
        set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS "/wd4456")
        # declaration hides previous declaration -- for picojson
        set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS "/wd4706")
    else()
        # re-enable old style casts -- for GLM
        set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS "-Wno-old-style-cast")
    endif()
endfunction()

add_dawn_sample(CHelloTriangle CHelloTriangle.cpp)
add_dawn_sample(CppHelloTriangle CppHelloTriangle.cpp)
add_dawn_sample(ComputeBoids ComputeBoids.cpp)
add_dawn_sample(Animometer Animometer.cpp)
add_dawn_sample(CubeReflection CubeReflection.cpp)

add_dawn_sample(glTFViewer glTFViewer/glTFViewer.cpp)
