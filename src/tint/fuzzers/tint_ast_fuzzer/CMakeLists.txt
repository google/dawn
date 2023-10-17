# Copyright 2021 The Dawn & Tint Authors
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

function(add_tint_ast_fuzzer NAME)
  add_executable(${NAME} ${NAME}.cc ${AST_FUZZER_SOURCES})
  target_link_libraries(${NAME} PRIVATE libtint_ast_fuzzer tint_lang_spirv_writer_helpers)
  tint_fuzzer_compile_options(${NAME})
  if(TINT_BUILD_SPV_READER OR TINT_BUILD_SPV_WRITER)
    tint_spvheaders_compile_options(${NAME})
    tint_spvtools_compile_options(${NAME})
  endif()
  target_compile_definitions(${NAME} PRIVATE CUSTOM_MUTATOR)
  target_include_directories(${NAME} PRIVATE ${CMAKE_BINARY_DIR})
  target_compile_options(${NAME} PRIVATE
        -Wno-conditional-uninitialized
        -Wno-covered-switch-default
        -Wno-shadow-field-in-constructor
        -Wno-shorten-64-to-32
        -Wno-undef)
endfunction()

set(PROTOBUF_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/protobufs/tint_ast_fuzzer.proto)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protobufs)

add_custom_command(
        OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/protobufs/tint_ast_fuzzer.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/protobufs/tint_ast_fuzzer.pb.h
        COMMAND
        "protobuf::protoc" -I=${CMAKE_CURRENT_SOURCE_DIR}/protobufs
        --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/protobufs ${PROTOBUF_SOURCES}
        DEPENDS ${PROTOBUF_SOURCES}
        COMMENT "Generate protobuf sources from proto definition file.")

set(LIBTINT_AST_FUZZER_SOURCES
        ../mersenne_twister_engine.h
        ../random_generator.h
        ../random_generator_engine.h
        expression_size.h
        jump_tracker.h
        mutation.h
        mutation_finder.h
        mutation_finders/change_binary_operators.h
        mutation_finders/change_unary_operators.h
        mutation_finders/delete_statements.h
        mutation_finders/replace_identifiers.h
        mutation_finders/wrap_unary_operators.h
        mutations/change_binary_operator.h
        mutations/change_unary_operator.h
        mutations/delete_statement.h
        mutations/replace_identifier.h
        mutations/wrap_unary_operator.h
        mutator.h
        node_id_map.h
        probability_context.h
        protobufs/tint_ast_fuzzer.h
        util.h
        ${CMAKE_CURRENT_BINARY_DIR}/protobufs/tint_ast_fuzzer.pb.h)

set(LIBTINT_AST_FUZZER_SOURCES ${LIBTINT_AST_FUZZER_SOURCES}
        ../mersenne_twister_engine.cc
        ../random_generator.cc
        ../random_generator_engine.cc
        expression_size.cc
        jump_tracker.cc
        mutation.cc
        mutation_finder.cc
        mutation_finders/change_binary_operators.cc
        mutation_finders/change_unary_operators.cc
        mutation_finders/delete_statements.cc
        mutation_finders/replace_identifiers.cc
        mutation_finders/wrap_unary_operators.cc
        mutations/change_binary_operator.cc
        mutations/change_unary_operator.cc
        mutations/delete_statement.cc
        mutations/replace_identifier.cc
        mutations/wrap_unary_operator.cc
        mutator.cc
        node_id_map.cc
        probability_context.cc
        ${CMAKE_CURRENT_BINARY_DIR}/protobufs/tint_ast_fuzzer.pb.cc)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/protobufs/tint_ast_fuzzer.pb.cc PROPERTIES COMPILE_FLAGS -w)

# Add static library target.
add_library(libtint_ast_fuzzer STATIC ${LIBTINT_AST_FUZZER_SOURCES})
target_link_libraries(libtint_ast_fuzzer protobuf::libprotobuf tint_api_sanitize_fuzzer)
tint_default_compile_options(libtint_ast_fuzzer)
target_include_directories(libtint_ast_fuzzer PRIVATE ${CMAKE_BINARY_DIR})
target_compile_options(libtint_ast_fuzzer PRIVATE
        -Wno-conditional-uninitialized
        -Wno-covered-switch-default
        -Wno-shadow-field-in-constructor
        -Wno-shorten-64-to-32
        -Wno-undef)

set(AST_FUZZER_SOURCES
        cli.cc
        cli.h
        fuzzer.cc
        override_cli_params.h
        ../tint_common_fuzzer.cc
        ../tint_common_fuzzer.h)

set_source_files_properties(fuzzer.cc PROPERTIES COMPILE_FLAGS -Wno-missing-prototypes)

# Add libfuzzer targets.
# Targets back-ends according to command line arguments.
add_tint_ast_fuzzer(tint_ast_fuzzer)
# Targets back-ends individually.
add_tint_ast_fuzzer(tint_ast_hlsl_writer_fuzzer)
add_tint_ast_fuzzer(tint_ast_msl_writer_fuzzer)
add_tint_ast_fuzzer(tint_ast_spv_writer_fuzzer)
add_tint_ast_fuzzer(tint_ast_wgsl_writer_fuzzer)

# Add tests.
if (${TINT_BUILD_TESTS})
    set(TEST_SOURCES
            expression_size_test.cc
            jump_tracker_test.cc
            mutations/change_binary_operator_test.cc
            mutations/change_unary_operator_test.cc
            mutations/delete_statement_test.cc
            mutations/replace_identifier_test.cc
            mutations/wrap_unary_operator_test.cc)

    add_executable(tint_ast_fuzzer_unittests ${TEST_SOURCES})

    target_include_directories(
            tint_ast_fuzzer_unittests PRIVATE ${gmock_SOURCE_DIR}/include)
    target_link_libraries(tint_ast_fuzzer_unittests gmock_main libtint_ast_fuzzer)
    tint_default_compile_options(tint_ast_fuzzer_unittests)
    target_compile_options(tint_ast_fuzzer_unittests PRIVATE
            -Wno-global-constructors
            -Wno-weak-vtables
            -Wno-conditional-uninitialized
            -Wno-covered-switch-default
            -Wno-shadow-field-in-constructor
            -Wno-shorten-64-to-32
            -Wno-undef)

    target_include_directories(tint_ast_fuzzer_unittests PRIVATE ${CMAKE_BINARY_DIR})

    add_test(NAME tint_ast_fuzzer_unittests COMMAND tint_ast_fuzzer_unittests)
endif ()
