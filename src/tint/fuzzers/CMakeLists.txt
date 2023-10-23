# Copyright 2020 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
  tint_fuzzer_compile_options(${NAME})
  if(TINT_BUILD_SPV_READER OR TINT_BUILD_SPV_WRITER)
    tint_spvheaders_compile_options(${NAME})
    tint_spvtools_compile_options(${NAME})
  endif()
  target_link_libraries(${NAME} PRIVATE tint_lang_spirv_writer_helpers)
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

if (${TINT_BUILD_AST_FUZZER})
  add_subdirectory(tint_ast_fuzzer)
endif()

if (${TINT_BUILD_REGEX_FUZZER})
  add_subdirectory(tint_regex_fuzzer)
endif()
