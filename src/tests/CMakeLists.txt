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

set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(UNITTESTS_DIR ${TESTS_DIR}/unittests)
set(VALIDATION_TESTS_DIR ${UNITTESTS_DIR}/validation)
add_executable(nxt_unittests
    ${UNITTESTS_DIR}/BitSetIteratorTests.cpp
    ${UNITTESTS_DIR}/CommandAllocatorTests.cpp
    ${UNITTESTS_DIR}/EnumClassBitmasksTests.cpp
    ${UNITTESTS_DIR}/MathTests.cpp
    ${UNITTESTS_DIR}/ObjectBaseTests.cpp
    ${UNITTESTS_DIR}/PerStageTests.cpp
    ${UNITTESTS_DIR}/RefCountedTests.cpp
    ${UNITTESTS_DIR}/ToBackendTests.cpp
    ${UNITTESTS_DIR}/WireTests.cpp
    ${VALIDATION_TESTS_DIR}/BufferValidationTest.cpp
    ${VALIDATION_TESTS_DIR}/DepthStencilStateValidationTest.cpp
    ${VALIDATION_TESTS_DIR}/ValidationTest.cpp
    ${VALIDATION_TESTS_DIR}/ValidationTest.h
    ${TESTS_DIR}/UnittestsMain.cpp
)
target_link_libraries(nxt_unittests gtest nxt_backend mock_nxt nxt_wire)
target_include_directories(nxt_unittests PRIVATE ${SRC_DIR})
SetCXX14(nxt_unittests)
