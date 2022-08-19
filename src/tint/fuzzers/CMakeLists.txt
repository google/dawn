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
    cli.cc
    cli.h
    data_builder.h
    fuzzer_init.cc
    fuzzer_init.h
    mersenne_twister_engine.cc
    mersenne_twister_engine.h
    random_generator.cc
    random_generator.h
    random_generator_engine.cc
    random_generator_engine.h
    shuffle_transform.cc
    shuffle_transform.h
    tint_common_fuzzer.cc
    tint_common_fuzzer.h
    tint_reader_writer_fuzzer.h
    transform_builder.h
    )
  target_link_libraries(${NAME} libtint-fuzz)
  tint_default_compile_options(${NAME})
  target_compile_options(${NAME} PRIVATE -Wno-missing-prototypes)
endfunction()

if (${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_WGSL_WRITER})
  add_tint_fuzzer(tint_wgsl_reader_wgsl_writer_fuzzer)
endif()

if (${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_SPV_WRITER})
  add_tint_fuzzer(tint_all_transforms_fuzzer)
  add_tint_fuzzer(tint_binding_remapper_fuzzer)
  add_tint_fuzzer(tint_concurrency_fuzzer)
  add_tint_fuzzer(tint_first_index_offset_fuzzer)
  add_tint_fuzzer(tint_renamer_fuzzer)
  add_tint_fuzzer(tint_robustness_fuzzer)
  add_tint_fuzzer(tint_single_entry_point_fuzzer)
  add_tint_fuzzer(tint_vertex_pulling_fuzzer)
  add_tint_fuzzer(tint_wgsl_reader_spv_writer_fuzzer)
endif()

if (${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_HLSL_WRITER})
  add_tint_fuzzer(tint_wgsl_reader_hlsl_writer_fuzzer)
endif()

if (${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_MSL_WRITER})
  add_tint_fuzzer(tint_wgsl_reader_msl_writer_fuzzer)
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

if (${TINT_BUILD_SPIRV_TOOLS_FUZZER})
  add_subdirectory(tint_spirv_tools_fuzzer)
endif()

if (${TINT_BUILD_AST_FUZZER})
  add_subdirectory(tint_ast_fuzzer)
endif()

if (${TINT_BUILD_REGEX_FUZZER})
  add_subdirectory(tint_regex_fuzzer)
endif()

if (${TINT_BUILD_WGSL_READER}
    AND ${TINT_BUILD_HLSL_WRITER}
    AND ${TINT_BUILD_MSL_WRITER}
    AND ${TINT_BUILD_SPV_WRITER}
    AND ${TINT_BUILD_WGSL_WRITER})
  add_executable(tint_black_box_fuzz_target
    mersenne_twister_engine.cc
    mersenne_twister_engine.h
    random_generator.cc
    random_generator.h
    random_generator_engine.cc
    random_generator_engine.h
    tint_black_box_fuzz_target.cc
    tint_common_fuzzer.cc
    tint_common_fuzzer.h
    )
  target_link_libraries(tint_black_box_fuzz_target libtint)
  tint_default_compile_options(tint_black_box_fuzz_target)
endif()
