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

Generate(
    LIB_NAME mock_nxt
    LIB_TYPE STATIC
    FOLDER "tests"
    PRINT_NAME libMockNXT
    COMMAND_LINE_ARGS
        ${GENERATOR_COMMON_ARGS}
        -T mock_nxt
)
target_include_directories(mock_nxt PUBLIC ${GENERATED_DIR})
target_link_libraries(mock_nxt nxt gtest)

set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(UNITTESTS_DIR ${TESTS_DIR}/unittests)
set(VALIDATION_TESTS_DIR ${UNITTESTS_DIR}/validation)
set(END2END_TESTS_DIR ${TESTS_DIR}/end2end)

list(APPEND UNITTEST_SOURCES
    ${UNITTESTS_DIR}/BitSetIteratorTests.cpp
    ${UNITTESTS_DIR}/CommandAllocatorTests.cpp
    ${UNITTESTS_DIR}/EnumClassBitmasksTests.cpp
    ${UNITTESTS_DIR}/ErrorTests.cpp
    ${UNITTESTS_DIR}/MathTests.cpp
    ${UNITTESTS_DIR}/ObjectBaseTests.cpp
    ${UNITTESTS_DIR}/PerStageTests.cpp
    ${UNITTESTS_DIR}/RefCountedTests.cpp
    ${UNITTESTS_DIR}/ResultTests.cpp
    ${UNITTESTS_DIR}/SerialQueueTests.cpp
    ${UNITTESTS_DIR}/ToBackendTests.cpp
    ${UNITTESTS_DIR}/WireTests.cpp
    ${VALIDATION_TESTS_DIR}/BindGroupValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/BlendStateValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/BufferValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/CommandBufferValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/ComputeValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/CopyCommandsValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/DepthStencilStateValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/DynamicStateCommandValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/InputStateValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/PushConstantsValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/RenderPassDescriptorValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/RenderPipelineValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/UsageValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/VertexBufferValidationTests.cpp
    ${VALIDATION_TESTS_DIR}/ValidationTest.cpp
    ${VALIDATION_TESTS_DIR}/ValidationTest.h
    ${TESTS_DIR}/UnittestsMain.cpp
)

if (NXT_ENABLE_D3D12)
    list(APPEND UNITTEST_SOURCES
        ${UNITTESTS_DIR}/d3d12/CopySplitTests.cpp
    )
endif()

add_executable(nxt_unittests ${UNITTEST_SOURCES})
target_link_libraries(nxt_unittests nxt_common gtest nxt_backend mock_nxt nxt_wire utils)
NXTInternalTarget("tests" nxt_unittests)

add_executable(nxt_end2end_tests
    ${END2END_TESTS_DIR}/BasicTests.cpp
    ${END2END_TESTS_DIR}/BufferTests.cpp
    ${END2END_TESTS_DIR}/BlendStateTests.cpp
    ${END2END_TESTS_DIR}/CopyTests.cpp
    ${END2END_TESTS_DIR}/DrawElementsTests.cpp
    ${END2END_TESTS_DIR}/DepthStencilStateTests.cpp
    ${END2END_TESTS_DIR}/IndexFormatTests.cpp
    ${END2END_TESTS_DIR}/InputStateTests.cpp
    ${END2END_TESTS_DIR}/PrimitiveTopologyTests.cpp
    ${END2END_TESTS_DIR}/PushConstantTests.cpp
    ${END2END_TESTS_DIR}/RenderPassLoadOpTests.cpp
    ${END2END_TESTS_DIR}/ScissorTests.cpp
    ${END2END_TESTS_DIR}/SamplerTests.cpp
    ${END2END_TESTS_DIR}/ViewportOrientationTests.cpp
    ${TESTS_DIR}/End2EndTestsMain.cpp
    ${TESTS_DIR}/NXTTest.cpp
    ${TESTS_DIR}/NXTTest.h
)
target_link_libraries(nxt_end2end_tests nxt_common nxt_wire gtest utils)
NXTInternalTarget("tests" nxt_end2end_tests)
