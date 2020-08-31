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

function(tint_spvtools_compile_options TARGET)
  # We'll use the optimizer for its nice SPIR-V in-memory representation
  target_link_libraries(${TARGET} SPIRV-Tools-opt SPIRV-Tools)

  # We'll be cheating: using internal interfaces to the SPIRV-Tools
  # optimizer.
  target_include_directories(${TARGET} PRIVATE
    ${spirv-tools_SOURCE_DIR}
    ${spirv-tools_BINARY_DIR}
  )

  if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    # The SPIRV-Tools code is conditioned against C++ and an older version of Clang.
    # Suppress warnings triggered in our current compilation environment.
    # TODO(dneto): Fix the issues upstream.
    target_compile_options(${TARGET} PRIVATE
      -Wno-newline-eof
      -Wno-sign-conversion
      -Wno-old-style-cast
      -Wno-weak-vtables
    )
  endif()
endfunction()

set(TINT_LIB_SRCS
  ../include/tint/tint.h
  ast/array_accessor_expression.cc
  ast/array_accessor_expression.h
  ast/as_expression.cc
  ast/as_expression.h
  ast/assignment_statement.cc
  ast/assignment_statement.h
  ast/binary_expression.cc
  ast/binary_expression.h
  ast/binding_decoration.cc
  ast/binding_decoration.h
  ast/block_statement.cc
  ast/block_statement.h
  ast/bool_literal.h
  ast/bool_literal.cc
  ast/break_statement.cc
  ast/break_statement.h
  ast/builtin.cc
  ast/builtin.h
  ast/builtin_decoration.cc
  ast/builtin_decoration.h
  ast/call_expression.cc
  ast/call_expression.h
  ast/call_statement.cc
  ast/call_statement.h
  ast/case_statement.cc
  ast/case_statement.h
  ast/cast_expression.cc
  ast/cast_expression.h
  ast/constructor_expression.cc
  ast/constructor_expression.h
  ast/continue_statement.cc
  ast/continue_statement.h
  ast/decorated_variable.cc
  ast/decorated_variable.h
  ast/discard_statement.cc
  ast/discard_statement.h
  ast/else_statement.cc
  ast/else_statement.h
  ast/entry_point.cc
  ast/entry_point.h
  ast/expression.cc
  ast/expression.h
  ast/fallthrough_statement.cc
  ast/fallthrough_statement.h
  ast/float_literal.cc
  ast/float_literal.h
  ast/function.cc
  ast/function.h
  ast/identifier_expression.cc
  ast/identifier_expression.h
  ast/if_statement.cc
  ast/if_statement.h
  ast/import.cc
  ast/import.h
  ast/int_literal.cc
  ast/int_literal.h
  ast/intrinsic.cc
  ast/intrinsic.h
  ast/literal.h
  ast/literal.cc
  ast/location_decoration.cc
  ast/location_decoration.h
  ast/loop_statement.cc
  ast/loop_statement.h
  ast/member_accessor_expression.cc
  ast/member_accessor_expression.h
  ast/module.cc
  ast/module.h
  ast/node.cc
  ast/node.h
  ast/null_literal.cc
  ast/null_literal.h
  ast/pipeline_stage.cc
  ast/pipeline_stage.h
  ast/return_statement.cc
  ast/return_statement.h
  ast/scalar_constructor_expression.cc
  ast/scalar_constructor_expression.h
  ast/set_decoration.cc
  ast/set_decoration.h
  ast/sint_literal.cc
  ast/sint_literal.h
  ast/statement.cc
  ast/statement.h
  ast/storage_class.cc
  ast/storage_class.h
  ast/struct_decoration.cc
  ast/struct_decoration.h
  ast/struct.cc
  ast/struct.h
  ast/struct_member.cc
  ast/struct_member.h
  ast/struct_member_decoration.cc
  ast/struct_member_decoration.h
  ast/struct_member_offset_decoration.cc
  ast/struct_member_offset_decoration.h
  ast/switch_statement.cc
  ast/switch_statement.h
  ast/transform/vertex_pulling_transform.cc
  ast/transform/vertex_pulling_transform.h
  ast/type_constructor_expression.h
  ast/type_constructor_expression.cc
  ast/type/alias_type.cc
  ast/type/alias_type.h
  ast/type/array_type.cc
  ast/type/array_type.h
  ast/type/bool_type.cc
  ast/type/bool_type.h
  ast/type/depth_texture_type.cc
  ast/type/depth_texture_type.h
  ast/type/f32_type.cc
  ast/type/f32_type.h
  ast/type/i32_type.cc
  ast/type/i32_type.h
  ast/type/matrix_type.cc
  ast/type/matrix_type.h
  ast/type/pointer_type.cc
  ast/type/pointer_type.h
  ast/type/sampled_texture_type.cc
  ast/type/sampled_texture_type.h
  ast/type/sampler_type.cc
  ast/type/sampler_type.h
  ast/type/storage_texture_type.cc
  ast/type/storage_texture_type.h
  ast/type/struct_type.cc
  ast/type/struct_type.h
  ast/type/texture_type.cc
  ast/type/texture_type.h
  ast/type/type.cc
  ast/type/type.h
  ast/type/u32_type.cc
  ast/type/u32_type.h
  ast/type/vector_type.cc
  ast/type/vector_type.h
  ast/type/void_type.cc
  ast/type/void_type.h
  ast/uint_literal.cc
  ast/uint_literal.h
  ast/unary_op.cc
  ast/unary_op.h
  ast/unary_op_expression.cc
  ast/unary_op_expression.h
  ast/variable.cc
  ast/variable.h
  ast/variable_decoration.cc
  ast/variable_decoration.h
  ast/variable_decl_statement.cc
  ast/variable_decl_statement.h
  context.h
  context.cc
  reader/reader.cc
  reader/reader.h
  scope_stack.h
  source.h
  type_determiner.cc
  type_determiner.h
  type_manager.cc
  type_manager.h
  validator.cc
  validator.h
  validator_impl.cc
  validator_impl.h
  validator_test_helper.cc
  validator_test_helper.h
  writer/text.cc
  writer/text.h
  writer/text_generator.cc
  writer/text_generator.h
  writer/writer.cc
  writer/writer.h
)

if(${TINT_BUILD_SPV_READER})
  list(APPEND TINT_LIB_SRCS
    reader/spirv/construct.h
    reader/spirv/construct.cc
    reader/spirv/enum_converter.h
    reader/spirv/enum_converter.cc
    reader/spirv/fail_stream.h
    reader/spirv/function.cc
    reader/spirv/function.h
    reader/spirv/namer.cc
    reader/spirv/namer.h
    reader/spirv/parser.cc
    reader/spirv/parser.h
    reader/spirv/parser_impl.cc
    reader/spirv/parser_impl.h
  )
endif()

if(${TINT_BUILD_WGSL_READER})
  list(APPEND TINT_LIB_SRCS
    reader/wgsl/lexer.cc
    reader/wgsl/lexer.h
    reader/wgsl/parser.cc
    reader/wgsl/parser.h
    reader/wgsl/parser_impl.cc
    reader/wgsl/parser_impl.h
    reader/wgsl/token.cc
    reader/wgsl/token.h
  )
endif()

if(${TINT_BUILD_SPV_WRITER})
  list(APPEND TINT_LIB_SRCS
    writer/spirv/binary_writer.cc
    writer/spirv/binary_writer.h
    writer/spirv/builder.cc
    writer/spirv/builder.h
    writer/spirv/function.cc
    writer/spirv/function.h
    writer/spirv/generator.cc
    writer/spirv/generator.h
    writer/spirv/instruction.cc
    writer/spirv/instruction.h
    writer/spirv/operand.cc
    writer/spirv/operand.h
  )
endif()

if(${TINT_BUILD_WGSL_WRITER})
  list(APPEND TINT_LIB_SRCS
    writer/wgsl/generator.cc
    writer/wgsl/generator.h
    writer/wgsl/generator_impl.cc
    writer/wgsl/generator_impl.h
  )
endif()

if(${TINT_BUILD_MSL_WRITER})
  list(APPEND TINT_LIB_SRCS
    writer/msl/generator.cc
    writer/msl/generator.h
    writer/msl/generator_impl.cc
    writer/msl/generator_impl.h
    writer/msl/namer.cc
    writer/msl/namer.h
  )
endif()

if(${TINT_BUILD_HLSL_WRITER})
  list(APPEND TINT_LIB_SRCS
    writer/hlsl/generator.cc
    writer/hlsl/generator.h
    writer/hlsl/generator_impl.cc
    writer/hlsl/generator_impl.h
    writer/hlsl/namer.cc
    writer/hlsl/namer.h
  )
endif()

set(TINT_TEST_SRCS
  ast/array_accessor_expression_test.cc
  ast/as_expression_test.cc
  ast/assignment_statement_test.cc
  ast/binding_decoration_test.cc
  ast/block_statement_test.cc
  ast/bool_literal_test.cc
  ast/break_statement_test.cc
  ast/builtin_decoration_test.cc
  ast/call_expression_test.cc
  ast/call_statement_test.cc
  ast/case_statement_test.cc
  ast/cast_expression_test.cc
  ast/continue_statement_test.cc
  ast/discard_statement_test.cc
  ast/decorated_variable_test.cc
  ast/else_statement_test.cc
  ast/entry_point_test.cc
  ast/expression_test.cc
  ast/fallthrough_statement_test.cc
  ast/float_literal_test.cc
  ast/function_test.cc
  ast/identifier_expression_test.cc
  ast/if_statement_test.cc
  ast/import_test.cc
  ast/int_literal_test.cc
  ast/location_decoration_test.cc
  ast/loop_statement_test.cc
  ast/member_accessor_expression_test.cc
  ast/module_test.cc
  ast/null_literal_test.cc
  ast/binary_expression_test.cc
  ast/return_statement_test.cc
  ast/scalar_constructor_expression_test.cc
  ast/set_decoration_test.cc
  ast/sint_literal_test.cc
  ast/struct_member_test.cc
  ast/struct_member_offset_decoration_test.cc
  ast/struct_test.cc
  ast/switch_statement_test.cc
  ast/transform/vertex_pulling_transform_test.cc
  ast/type/alias_type_test.cc
  ast/type/array_type_test.cc
  ast/type/bool_type_test.cc
  ast/type/depth_texture_type_test.cc
  ast/type/f32_type_test.cc
  ast/type/i32_type_test.cc
  ast/type/matrix_type_test.cc
  ast/type/pointer_type_test.cc
  ast/type/sampled_texture_type_test.cc
  ast/type/sampler_type_test.cc
  ast/type/storage_texture_type_test.cc
  ast/type/struct_type_test.cc
  ast/type/u32_type_test.cc
  ast/type/vector_type_test.cc
  ast/type_constructor_expression_test.cc
  ast/uint_literal_test.cc
  ast/unary_op_expression_test.cc
  ast/variable_decl_statement_test.cc
  ast/variable_test.cc
  scope_stack_test.cc
  type_determiner_test.cc
  type_manager_test.cc
  validator_test.cc
  validator_function_test.cc
)

## Tint library
add_library(libtint ${TINT_LIB_SRCS})
tint_default_compile_options(libtint)
set_target_properties(libtint PROPERTIES OUTPUT_NAME "tint")

if(${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
  tint_spvtools_compile_options(libtint)
endif()

if(${TINT_BUILD_SPV_READER})
  list(APPEND TINT_TEST_SRCS
    reader/spirv/enum_converter_test.cc
    reader/spirv/fail_stream_test.cc
    reader/spirv/function_arithmetic_test.cc
    reader/spirv/function_bit_test.cc
    reader/spirv/function_cfg_test.cc
    reader/spirv/function_call_test.cc
    reader/spirv/function_composite_test.cc
    reader/spirv/function_conversion_test.cc
    reader/spirv/function_decl_test.cc
    reader/spirv/function_glsl_std_450_test.cc
    reader/spirv/function_logical_test.cc
    reader/spirv/function_memory_test.cc
    reader/spirv/function_misc_test.cc
    reader/spirv/function_var_test.cc
    reader/spirv/namer_test.cc
    reader/spirv/parser_impl_convert_member_decoration_test.cc
    reader/spirv/parser_impl_convert_type_test.cc
    reader/spirv/parser_impl_entry_point_test.cc
    reader/spirv/parser_impl_function_decl_test.cc
    reader/spirv/parser_impl_get_decorations_test.cc
    reader/spirv/parser_impl_import_test.cc
    reader/spirv/parser_impl_module_var_test.cc
    reader/spirv/parser_impl_named_types_test.cc
    reader/spirv/parser_impl_test.cc
    reader/spirv/parser_impl_user_name_test.cc
    reader/spirv/parser_test.cc
    reader/spirv/spirv_tools_helpers_test.cc
    reader/spirv/spirv_tools_helpers_test.h
  )
endif()

if(${TINT_BUILD_WGSL_READER})
  list(APPEND TINT_TEST_SRCS
    reader/wgsl/lexer_test.cc
    reader/wgsl/parser_test.cc
    reader/wgsl/parser_impl_additive_expression_test.cc
    reader/wgsl/parser_impl_and_expression_test.cc
    reader/wgsl/parser_impl_argument_expression_list_test.cc
    reader/wgsl/parser_impl_assignment_stmt_test.cc
    reader/wgsl/parser_impl_body_stmt_test.cc
    reader/wgsl/parser_impl_break_stmt_test.cc
    reader/wgsl/parser_impl_call_stmt_test.cc
    reader/wgsl/parser_impl_case_body_test.cc
    reader/wgsl/parser_impl_const_expr_test.cc
    reader/wgsl/parser_impl_const_literal_test.cc
    reader/wgsl/parser_impl_continue_stmt_test.cc
    reader/wgsl/parser_impl_continuing_stmt_test.cc
    reader/wgsl/parser_impl_else_stmt_test.cc
    reader/wgsl/parser_impl_elseif_stmt_test.cc
    reader/wgsl/parser_impl_entry_point_decl_test.cc
    reader/wgsl/parser_impl_equality_expression_test.cc
    reader/wgsl/parser_impl_exclusive_or_expression_test.cc
    reader/wgsl/parser_impl_for_stmt_test.cc
    reader/wgsl/parser_impl_function_decl_test.cc
    reader/wgsl/parser_impl_function_header_test.cc
    reader/wgsl/parser_impl_function_type_decl_test.cc
    reader/wgsl/parser_impl_global_constant_decl_test.cc
    reader/wgsl/parser_impl_global_decl_test.cc
    reader/wgsl/parser_impl_global_variable_decl_test.cc
    reader/wgsl/parser_impl_if_stmt_test.cc
    reader/wgsl/parser_impl_import_decl_test.cc
    reader/wgsl/parser_impl_inclusive_or_expression_test.cc
    reader/wgsl/parser_impl_logical_and_expression_test.cc
    reader/wgsl/parser_impl_logical_or_expression_test.cc
    reader/wgsl/parser_impl_loop_stmt_test.cc
    reader/wgsl/parser_impl_multiplicative_expression_test.cc
    reader/wgsl/parser_impl_param_list_test.cc
    reader/wgsl/parser_impl_paren_rhs_stmt_test.cc
    reader/wgsl/parser_impl_pipeline_stage_test.cc
    reader/wgsl/parser_impl_postfix_expression_test.cc
    reader/wgsl/parser_impl_primary_expression_test.cc
    reader/wgsl/parser_impl_relational_expression_test.cc
    reader/wgsl/parser_impl_shift_expression_test.cc
    reader/wgsl/parser_impl_statement_test.cc
    reader/wgsl/parser_impl_statements_test.cc
    reader/wgsl/parser_impl_storage_class_test.cc
    reader/wgsl/parser_impl_struct_body_decl_test.cc
    reader/wgsl/parser_impl_struct_decl_test.cc
    reader/wgsl/parser_impl_struct_decoration_decl_test.cc
    reader/wgsl/parser_impl_struct_decoration_test.cc
    reader/wgsl/parser_impl_struct_member_decoration_decl_test.cc
    reader/wgsl/parser_impl_struct_member_decoration_test.cc
    reader/wgsl/parser_impl_struct_member_test.cc
    reader/wgsl/parser_impl_switch_body_test.cc
    reader/wgsl/parser_impl_switch_stmt_test.cc
    reader/wgsl/parser_impl_test.cc
    reader/wgsl/parser_impl_test_helper.cc
    reader/wgsl/parser_impl_test_helper.h
    reader/wgsl/parser_impl_type_alias_test.cc
    reader/wgsl/parser_impl_type_decl_test.cc
    reader/wgsl/parser_impl_unary_expression_test.cc
    reader/wgsl/parser_impl_variable_decl_test.cc
    reader/wgsl/parser_impl_variable_decoration_list_test.cc
    reader/wgsl/parser_impl_variable_decoration_test.cc
    reader/wgsl/parser_impl_variable_ident_decl_test.cc
    reader/wgsl/parser_impl_variable_stmt_test.cc
    reader/wgsl/parser_impl_variable_storage_decoration_test.cc
    reader/wgsl/token_test.cc
  )
endif()

if(${TINT_BUILD_SPV_WRITER})
  list(APPEND TINT_TEST_SRCS
    writer/spirv/binary_writer_test.cc
    writer/spirv/builder_accessor_expression_test.cc
    writer/spirv/builder_as_expression_test.cc
    writer/spirv/builder_assign_test.cc
    writer/spirv/builder_binary_expression_test.cc
    writer/spirv/builder_block_test.cc
    writer/spirv/builder_call_test.cc
    writer/spirv/builder_cast_expression_test.cc
    writer/spirv/builder_constructor_expression_test.cc
    writer/spirv/builder_discard_test.cc
    writer/spirv/builder_entry_point_test.cc
    writer/spirv/builder_format_conversion_test.cc
    writer/spirv/builder_function_test.cc
    writer/spirv/builder_function_variable_test.cc
    writer/spirv/builder_global_variable_test.cc
    writer/spirv/builder_ident_expression_test.cc
    writer/spirv/builder_if_test.cc
    writer/spirv/builder_intrinsic_test.cc
    writer/spirv/builder_literal_test.cc
    writer/spirv/builder_loop_test.cc
    writer/spirv/builder_return_test.cc
    writer/spirv/builder_switch_test.cc
    writer/spirv/builder_test.cc
    writer/spirv/builder_type_test.cc
    writer/spirv/builder_unary_op_expression_test.cc
    writer/spirv/instruction_test.cc
    writer/spirv/operand_test.cc
    writer/spirv/spv_dump.cc
    writer/spirv/spv_dump.h
  )
endif()

if(${TINT_BUILD_WGSL_WRITER})
  list(APPEND TINT_TEST_SRCS
    writer/wgsl/generator_impl_test.cc
    writer/wgsl/generator_impl_alias_type_test.cc
    writer/wgsl/generator_impl_array_accessor_test.cc
    writer/wgsl/generator_impl_as_test.cc
    writer/wgsl/generator_impl_assign_test.cc
    writer/wgsl/generator_impl_binary_test.cc
    writer/wgsl/generator_impl_block_test.cc
    writer/wgsl/generator_impl_break_test.cc
    writer/wgsl/generator_impl_call_test.cc
    writer/wgsl/generator_impl_case_test.cc
    writer/wgsl/generator_impl_cast_test.cc
    writer/wgsl/generator_impl_constructor_test.cc
    writer/wgsl/generator_impl_continue_test.cc
    writer/wgsl/generator_impl_discard_test.cc
    writer/wgsl/generator_impl_entry_point_test.cc
    writer/wgsl/generator_impl_fallthrough_test.cc
    writer/wgsl/generator_impl_function_test.cc
    writer/wgsl/generator_impl_identifier_test.cc
    writer/wgsl/generator_impl_if_test.cc
    writer/wgsl/generator_impl_import_test.cc
    writer/wgsl/generator_impl_loop_test.cc
    writer/wgsl/generator_impl_member_accessor_test.cc
    writer/wgsl/generator_impl_return_test.cc
    writer/wgsl/generator_impl_switch_test.cc
    writer/wgsl/generator_impl_type_test.cc
    writer/wgsl/generator_impl_unary_op_test.cc
    writer/wgsl/generator_impl_variable_decl_statement_test.cc
    writer/wgsl/generator_impl_variable_test.cc
  )
endif()

if(${TINT_BUILD_MSL_WRITER})
  list(APPEND TINT_TEST_SRCS
    writer/msl/generator_impl_alias_type_test.cc
    writer/msl/generator_impl_array_accessor_test.cc
    writer/msl/generator_impl_as_test.cc
    writer/msl/generator_impl_assign_test.cc
    writer/msl/generator_impl_binary_test.cc
    writer/msl/generator_impl_block_test.cc
    writer/msl/generator_impl_break_test.cc
    writer/msl/generator_impl_call_test.cc
    writer/msl/generator_impl_case_test.cc
    writer/msl/generator_impl_cast_test.cc
    writer/msl/generator_impl_constructor_test.cc
    writer/msl/generator_impl_continue_test.cc
    writer/msl/generator_impl_discard_test.cc
    writer/msl/generator_impl_entry_point_test.cc
    writer/msl/generator_impl_function_test.cc
    writer/msl/generator_impl_identifier_test.cc
    writer/msl/generator_impl_if_test.cc
    writer/msl/generator_impl_import_test.cc
    writer/msl/generator_impl_intrinsic_test.cc
    writer/msl/generator_impl_loop_test.cc
    writer/msl/generator_impl_member_accessor_test.cc
    writer/msl/generator_impl_module_constant_test.cc
    writer/msl/generator_impl_return_test.cc
    writer/msl/generator_impl_switch_test.cc
    writer/msl/generator_impl_test.cc
    writer/msl/generator_impl_type_test.cc
    writer/msl/generator_impl_unary_op_test.cc
    writer/msl/generator_impl_variable_decl_statement_test.cc
    writer/msl/namer_test.cc
  )
endif()

if (${TINT_BUILD_HLSL_WRITER})
  list(APPEND TINT_TEST_SRCS
    writer/hlsl/generator_impl_alias_type_test.cc
    writer/hlsl/generator_impl_array_accessor_test.cc
    writer/hlsl/generator_impl_as_test.cc
    writer/hlsl/generator_impl_assign_test.cc
    writer/hlsl/generator_impl_binary_test.cc
    writer/hlsl/generator_impl_block_test.cc
    writer/hlsl/generator_impl_break_test.cc
    writer/hlsl/generator_impl_call_test.cc
    writer/hlsl/generator_impl_case_test.cc
    writer/hlsl/generator_impl_cast_test.cc
    writer/hlsl/generator_impl_constructor_test.cc
    writer/hlsl/generator_impl_continue_test.cc
    writer/hlsl/generator_impl_discard_test.cc
    writer/hlsl/generator_impl_entry_point_test.cc
    writer/hlsl/generator_impl_function_test.cc
    writer/hlsl/generator_impl_identifier_test.cc
    writer/hlsl/generator_impl_if_test.cc
    writer/hlsl/generator_impl_intrinsic_test.cc
    writer/hlsl/generator_impl_import_test.cc
    writer/hlsl/generator_impl_loop_test.cc
    writer/hlsl/generator_impl_member_accessor_test.cc
    writer/hlsl/generator_impl_module_constant_test.cc
    writer/hlsl/generator_impl_return_test.cc
    writer/hlsl/generator_impl_switch_test.cc
    writer/hlsl/generator_impl_test.cc
    writer/hlsl/generator_impl_type_test.cc
    writer/hlsl/generator_impl_unary_op_test.cc
    writer/hlsl/generator_impl_variable_decl_statement_test.cc
    writer/hlsl/namer_test.cc
    writer/hlsl/test_helper.h
  )
endif()

add_executable(tint_unittests ${TINT_TEST_SRCS})

if(NOT MSVC)
  target_compile_options(tint_unittests PRIVATE
    -Wno-global-constructors
    -Wno-weak-vtables
  )
endif()

## Test executable
target_include_directories(
    tint_unittests PRIVATE ${gmock_SOURCE_DIR}/include)
target_link_libraries(tint_unittests libtint gmock_main)
tint_default_compile_options(tint_unittests)

if(${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
  tint_spvtools_compile_options(tint_unittests)
endif()

add_test(NAME tint_unittests COMMAND tint_unittests)
