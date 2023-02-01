# Copyright 2022 The Tint and Dawn Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project(turbo-cov)

if (LLVM_SOURCE_DIR)
    set(LLVM_INCLUDE_TESTS OFF)
    set(LLVM_TARGETS_TO_BUILD "X86") # At least one target needs to be provided
    add_subdirectory("${LLVM_SOURCE_DIR}" "llvm" EXCLUDE_FROM_ALL)

    add_executable(turbo-cov
        main.cpp
    )

    set_target_properties(turbo-cov PROPERTIES
        FOLDER "Tests"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        INCLUDE_DIRECTORIES "${LLVM_BINARY_DIR}/include;${LLVM_SOURCE_DIR}/include"
    )

    target_link_libraries(turbo-cov
        PRIVATE
            LLVMProfileData
            LLVMCoverage
    )

    common_compile_options(turbo-cov)
else (LLVM_SOURCE_DIR)
    message("not building turbo-cov as LLVM_SOURCE_DIR is undefined")
endif (LLVM_SOURCE_DIR)
