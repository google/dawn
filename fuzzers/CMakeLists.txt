# Copyright 2020 The Tint Authors.
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

function(add_tint_fuzzer NAME)
  add_executable(${NAME}
    ${NAME}.cc
    tint_common_fuzzer.cc)
  target_link_libraries(${NAME} libtint-fuzz)
  tint_default_compile_options(${NAME})
  target_compile_options(${NAME} PRIVATE -Wno-missing-prototypes)
endfunction()

if (${TINT_BUILD_WGSL_READER})
  add_tint_fuzzer(tint_wgsl_reader_fuzzer)
endif()

if (${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_WGSL_WRITER})
  add_tint_fuzzer(tint_wgsl_reader_wgsl_writer_fuzzer)
endif()

if (${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_SPV_WRITER})
  add_tint_fuzzer(tint_all_transforms_fuzzer)
  add_tint_fuzzer(tint_binding_remapper_fuzzer)
  add_tint_fuzzer(tint_bound_array_accessors_fuzzer)
  add_tint_fuzzer(tint_first_index_offset_fuzzer)
  add_tint_fuzzer(tint_inspector_fuzzer)
  add_tint_fuzzer(tint_renamer_fuzzer)
  add_tint_fuzzer(tint_single_entry_point_fuzzer)
  add_tint_fuzzer(tint_spirv_transform_fuzzer)
  add_tint_fuzzer(tint_vertex_pulling_fuzzer)
  add_tint_fuzzer(tint_wgsl_reader_spv_writer_fuzzer)
endif()

if (${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_HLSL_WRITER})
  add_tint_fuzzer(tint_wgsl_reader_hlsl_writer_fuzzer)
  add_tint_fuzzer(tint_hlsl_transform_fuzzer)
endif()

if (${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_MSL_WRITER})
  add_tint_fuzzer(tint_wgsl_reader_msl_writer_fuzzer)
  add_tint_fuzzer(tint_msl_transform_fuzzer)
endif()

if (${TINT_BUILD_SPV_READER})
  add_tint_fuzzer(tint_spv_reader_fuzzer)
endif()

if (${TINT_BUILD_SPV_READER} AND ${TINT_BUILD_WGSL_WRITER})
  add_tint_fuzzer(tint_spv_reader_wgsl_writer_fuzzer)
endif()

if (${TINT_BUILD_SPV_READER} AND ${TINT_BUILD_SPV_WRITER})
  add_tint_fuzzer(tint_spv_reader_spv_writer_fuzzer)
endif()

if (${TINT_BUILD_SPV_READER} AND ${TINT_BUILD_HLSL_WRITER})
  add_tint_fuzzer(tint_spv_reader_hlsl_writer_fuzzer)
endif()

if (${TINT_BUILD_SPV_READER} AND ${TINT_BUILD_MSL_WRITER})
  add_tint_fuzzer(tint_spv_reader_msl_writer_fuzzer)
endif()


if (${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_WGSL_WRITER})
  add_tint_fuzzer(tint_ast_clone_fuzzer)
endif()
