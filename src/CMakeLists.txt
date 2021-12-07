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
  ast/access.cc
  ast/access.h
  ast/alias.cc
  ast/alias.h
  ast/index_accessor_expression.cc
  ast/index_accessor_expression.h
  ast/array.cc
  ast/array.h
  ast/assignment_statement.cc
  ast/assignment_statement.h
  ast/atomic.cc
  ast/atomic.h
  ast/binary_expression.cc
  ast/binary_expression.h
  ast/binding_decoration.cc
  ast/binding_decoration.h
  ast/bitcast_expression.cc
  ast/bitcast_expression.h
  ast/block_statement.cc
  ast/block_statement.h
  ast/bool_literal_expression.cc
  ast/bool_literal_expression.h
  ast/bool.cc
  ast/bool.h
  ast/break_statement.cc
  ast/break_statement.h
  ast/builtin_decoration.cc
  ast/builtin_decoration.h
  ast/builtin.cc
  ast/builtin.h
  ast/call_expression.cc
  ast/call_expression.h
  ast/call_statement.cc
  ast/call_statement.h
  ast/case_statement.cc
  ast/case_statement.h
  ast/continue_statement.cc
  ast/continue_statement.h
  ast/decoration.cc
  ast/decoration.h
  ast/depth_multisampled_texture.cc
  ast/depth_multisampled_texture.h
  ast/disable_validation_decoration.cc
  ast/disable_validation_decoration.h
  ast/depth_texture.cc
  ast/depth_texture.h
  ast/discard_statement.cc
  ast/discard_statement.h
  ast/else_statement.cc
  ast/else_statement.h
  ast/expression.cc
  ast/expression.h
  ast/external_texture.cc
  ast/external_texture.h
  ast/f32.cc
  ast/f32.h
  ast/fallthrough_statement.cc
  ast/fallthrough_statement.h
  ast/float_literal_expression.cc
  ast/float_literal_expression.h
  ast/for_loop_statement.cc
  ast/for_loop_statement.h
  ast/function.cc
  ast/function.h
  ast/group_decoration.cc
  ast/group_decoration.h
  ast/i32.cc
  ast/i32.h
  ast/identifier_expression.cc
  ast/identifier_expression.h
  ast/if_statement.cc
  ast/if_statement.h
  ast/int_literal_expression.cc
  ast/int_literal_expression.h
  ast/internal_decoration.cc
  ast/internal_decoration.h
  ast/interpolate_decoration.cc
  ast/interpolate_decoration.h
  ast/invariant_decoration.cc
  ast/invariant_decoration.h
  ast/literal_expression.cc
  ast/literal_expression.h
  ast/location_decoration.cc
  ast/location_decoration.h
  ast/loop_statement.cc
  ast/loop_statement.h
  ast/matrix.cc
  ast/matrix.h
  ast/member_accessor_expression.cc
  ast/member_accessor_expression.h
  ast/module.cc
  ast/module.h
  ast/multisampled_texture.cc
  ast/multisampled_texture.h
  ast/node.cc
  ast/node.h
  ast/override_decoration.cc
  ast/override_decoration.h
  ast/phony_expression.cc
  ast/phony_expression.h
  ast/pipeline_stage.cc
  ast/pipeline_stage.h
  ast/pointer.cc
  ast/pointer.h
  ast/return_statement.cc
  ast/return_statement.h
  ast/sampled_texture.cc
  ast/sampled_texture.h
  ast/sampler.cc
  ast/sampler.h
  ast/sint_literal_expression.cc
  ast/sint_literal_expression.h
  ast/stage_decoration.cc
  ast/stage_decoration.h
  ast/statement.cc
  ast/statement.h
  ast/storage_class.cc
  ast/storage_class.h
  ast/storage_texture.cc
  ast/storage_texture.h
  ast/stride_decoration.cc
  ast/stride_decoration.h
  ast/struct_block_decoration.cc
  ast/struct_block_decoration.h
  ast/struct_member_align_decoration.cc
  ast/struct_member_align_decoration.h
  ast/struct_member_offset_decoration.cc
  ast/struct_member_offset_decoration.h
  ast/struct_member_size_decoration.cc
  ast/struct_member_size_decoration.h
  ast/struct_member.cc
  ast/struct_member.h
  ast/struct.cc
  ast/struct.h
  ast/switch_statement.cc
  ast/switch_statement.h
  ast/texture.cc
  ast/texture.h
  ast/traverse_expressions.h
  ast/type_name.cc
  ast/type_name.h
  ast/ast_type.cc  # TODO(bclayton) - rename to type.cc
  ast/type.h
  ast/type_decl.cc
  ast/type_decl.h
  ast/type_name.cc
  ast/type_name.h
  ast/u32.cc
  ast/u32.h
  ast/uint_literal_expression.cc
  ast/uint_literal_expression.h
  ast/unary_op_expression.cc
  ast/unary_op_expression.h
  ast/unary_op.cc
  ast/unary_op.h
  ast/variable_decl_statement.cc
  ast/variable_decl_statement.h
  ast/variable.cc
  ast/variable.h
  ast/vector.cc
  ast/vector.h
  ast/void.cc
  ast/void.h
  ast/workgroup_decoration.cc
  ast/workgroup_decoration.h
  block_allocator.h
  castable.cc
  castable.h
  clone_context.cc
  clone_context.h
  debug.cc
  debug.h
  demangler.cc
  demangler.h
  diagnostic/diagnostic.cc
  diagnostic/diagnostic.h
  diagnostic/formatter.cc
  diagnostic/formatter.h
  diagnostic/printer.cc
  diagnostic/printer.h
  inspector/entry_point.cc
  inspector/entry_point.h
  inspector/inspector.cc
  inspector/inspector.h
  inspector/resource_binding.cc
  inspector/resource_binding.h
  inspector/sampler_texture_pair.h
  inspector/scalar.cc
  inspector/scalar.h
  intrinsic_table.cc
  intrinsic_table.h
  intrinsic_table.inl
  program_builder.cc
  program_builder.h
  program_id.cc
  program_id.h
  program.cc
  program.h
  reader/reader.cc
  reader/reader.h
  resolver/dependency_graph.cc
  resolver/dependency_graph.h
  resolver/resolver.cc
  resolver/resolver_constants.cc
  resolver/resolver_validation.cc
  resolver/resolver.h
  scope_stack.h
  sem/array.cc
  sem/array.h
  sem/atomic_type.cc
  sem/atomic_type.h
  sem/behavior.cc
  sem/behavior.h
  sem/binding_point.h
  sem/block_statement.cc
  sem/block_statement.h
  sem/call_target.cc
  sem/call_target.h
  sem/call.cc
  sem/call.h
  sem/constant.cc
  sem/constant.h
  sem/depth_multisampled_texture_type.cc
  sem/depth_multisampled_texture_type.h
  sem/expression.cc
  sem/expression.h
  sem/function.cc
  sem/info.cc
  sem/info.h
  sem/intrinsic_type.cc
  sem/intrinsic_type.h
  sem/intrinsic.cc
  sem/intrinsic.h
  sem/member_accessor_expression.cc
  sem/parameter_usage.cc
  sem/parameter_usage.h
  sem/pipeline_stage_set.h
  sem/node.cc
  sem/node.h
  sem/statement.cc
  sem/struct.cc
  sem/type_mappings.h
  sem/variable.cc
  source.cc
  source.h
  symbol_table.cc
  symbol_table.h
  symbol.cc
  symbol.h
  traits.h
  transform/add_empty_entry_point.cc
  transform/add_empty_entry_point.h
  transform/array_length_from_uniform.cc
  transform/array_length_from_uniform.h
  transform/binding_remapper.cc
  transform/binding_remapper.h
  transform/calculate_array_length.cc
  transform/calculate_array_length.h
  transform/canonicalize_entry_point_io.cc
  transform/canonicalize_entry_point_io.h
  transform/decompose_memory_access.cc
  transform/decompose_memory_access.h
  transform/decompose_strided_matrix.cc
  transform/decompose_strided_matrix.h
  transform/external_texture_transform.cc
  transform/external_texture_transform.h
  transform/first_index_offset.cc
  transform/first_index_offset.h
  transform/fold_constants.cc
  transform/fold_constants.h
  transform/fold_trivial_single_use_lets.cc
  transform/fold_trivial_single_use_lets.h
  transform/for_loop_to_loop.cc
  transform/for_loop_to_loop.h
  transform/glsl.cc
  transform/glsl.h
  transform/loop_to_for_loop.cc
  transform/loop_to_for_loop.h
  transform/manager.cc
  transform/manager.h
  transform/module_scope_var_to_entry_point_param.cc
  transform/module_scope_var_to_entry_point_param.h
  transform/multiplanar_external_texture.cc
  transform/multiplanar_external_texture.h
  transform/num_workgroups_from_uniform.cc
  transform/num_workgroups_from_uniform.h
  transform/pad_array_elements.cc
  transform/pad_array_elements.h
  transform/promote_initializers_to_const_var.cc
  transform/promote_initializers_to_const_var.h
  transform/remove_phonies.cc
  transform/remove_phonies.h
  transform/remove_unreachable_statements.cc
  transform/remove_unreachable_statements.h
  transform/renamer.cc
  transform/renamer.h
  transform/robustness.cc
  transform/robustness.h
  transform/simplify_pointers.cc
  transform/simplify_pointers.h
  transform/single_entry_point.cc
  transform/single_entry_point.h
  transform/transform.cc
  transform/transform.h
  transform/unshadow.cc
  transform/unshadow.h
  transform/vectorize_scalar_matrix_constructors.cc
  transform/vectorize_scalar_matrix_constructors.h
  transform/vertex_pulling.cc
  transform/vertex_pulling.h
  transform/wrap_arrays_in_structs.cc
  transform/wrap_arrays_in_structs.h
  transform/zero_init_workgroup_memory.cc
  transform/zero_init_workgroup_memory.h
  sem/bool_type.cc
  sem/bool_type.h
  sem/depth_texture_type.cc
  sem/depth_texture_type.h
  sem/external_texture_type.cc
  sem/external_texture_type.h
  sem/f32_type.cc
  sem/f32_type.h
  sem/for_loop_statement.cc
  sem/for_loop_statement.h
  sem/i32_type.cc
  sem/i32_type.h
  sem/if_statement.cc
  sem/if_statement.h
  sem/loop_statement.cc
  sem/loop_statement.h
  sem/matrix_type.cc
  sem/matrix_type.h
  sem/multisampled_texture_type.cc
  sem/multisampled_texture_type.h
  sem/pointer_type.cc
  sem/pointer_type.h
  sem/reference_type.cc
  sem/reference_type.h
  sem/sampled_texture_type.cc
  sem/sampled_texture_type.h
  sem/sampler_type.cc
  sem/sampler_type.h
  sem/storage_texture_type.cc
  sem/storage_texture_type.h
  sem/switch_statement.cc
  sem/switch_statement.h
  sem/texture_type.cc
  sem/texture_type.h
  sem/type_constructor.cc
  sem/type_constructor.h
  sem/type_conversion.cc
  sem/type_conversion.h
  sem/type.cc
  sem/type.h
  sem/type_manager.cc
  sem/type_manager.h
  sem/u32_type.cc
  sem/u32_type.h
  sem/vector_type.cc
  sem/vector_type.h
  sem/void_type.cc
  sem/void_type.h
  utils/enum_set.h
  utils/hash.h
  utils/map.h
  utils/math.h
  utils/scoped_assignment.h
  utils/string.h
  utils/unique_vector.h
  writer/append_vector.cc
  writer/append_vector.h
  writer/array_length_from_uniform_options.cc
  writer/array_length_from_uniform_options.h
  writer/float_to_string.cc
  writer/float_to_string.h
  writer/text_generator.cc
  writer/text_generator.h
  writer/text.cc
  writer/text.h
  writer/writer.cc
  writer/writer.h
)

if(UNIX)
  list(APPEND TINT_LIB_SRCS diagnostic/printer_linux.cc)
elseif(WIN32)
  list(APPEND TINT_LIB_SRCS diagnostic/printer_windows.cc)
else()
  list(APPEND TINT_LIB_SRCS diagnostic/printer_other.cc)
endif()

if(${TINT_BUILD_SPV_READER})
  list(APPEND TINT_LIB_SRCS
    reader/spirv/construct.h
    reader/spirv/construct.cc
    reader/spirv/entry_point_info.h
    reader/spirv/entry_point_info.cc
    reader/spirv/enum_converter.h
    reader/spirv/enum_converter.cc
    reader/spirv/fail_stream.h
    reader/spirv/function.cc
    reader/spirv/function.h
    reader/spirv/namer.cc
    reader/spirv/namer.h
    reader/spirv/parser_type.cc
    reader/spirv/parser_type.h
    reader/spirv/parser.cc
    reader/spirv/parser.h
    reader/spirv/parser_impl.cc
    reader/spirv/parser_impl.h
    reader/spirv/usage.cc
    reader/spirv/usage.h
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
    reader/wgsl/parser_impl_detail.h
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
    writer/spirv/scalar_constant.h
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
  )
endif()

if(${TINT_BUILD_GLSL_WRITER})
  list(APPEND TINT_LIB_SRCS
    writer/glsl/generator.cc
    writer/glsl/generator.h
    writer/glsl/generator_impl.cc
    writer/glsl/generator_impl.h
  )
endif()

if(${TINT_BUILD_HLSL_WRITER})
  list(APPEND TINT_LIB_SRCS
    writer/hlsl/generator.cc
    writer/hlsl/generator.h
    writer/hlsl/generator_impl.cc
    writer/hlsl/generator_impl.h
  )
endif()

## Tint IO utilities. Used by tint_val.
add_library(tint_utils_io
  utils/io/command_${TINT_OS_CC_SUFFIX}.cc
  utils/io/command.h
  utils/io/tmpfile_${TINT_OS_CC_SUFFIX}.cc
  utils/io/tmpfile.h
)
tint_default_compile_options(tint_utils_io)

## Tint validation utilities. Used by tests and the tint executable.
add_library(tint_val
  val/hlsl.cc
  val/msl.cc
  val/val.h
)

# If we're building on mac / ios and we have CoreGraphics, then we can use the
# metal API to validate our shaders. This is roughly 4x faster than invoking
# the metal shader compiler executable.
if(APPLE)
  find_library(LIB_CORE_GRAPHICS CoreGraphics)
  if(LIB_CORE_GRAPHICS)
    target_sources(tint_val PRIVATE "val/msl_metal.mm")
    target_compile_definitions(tint_val PUBLIC "-DTINT_ENABLE_MSL_VALIDATION_USING_METAL_API=1")
    target_compile_options(tint_val PRIVATE "-fmodules" "-fcxx-modules")
    target_link_options(tint_val PUBLIC "-framework" "CoreGraphics")
  endif()
endif()

tint_default_compile_options(tint_val)
target_link_libraries(tint_val tint_utils_io)

## Tint library
add_library(libtint ${TINT_LIB_SRCS})
tint_default_compile_options(libtint)
if (${COMPILER_IS_LIKE_GNU})
  target_compile_options(libtint PRIVATE -fvisibility=hidden)
endif()
if (${TINT_SYMBOL_STORE_DEBUG_NAME})
    target_compile_definitions(libtint PUBLIC "TINT_SYMBOL_STORE_DEBUG_NAME=1")
endif()
set_target_properties(libtint PROPERTIES OUTPUT_NAME "tint")

if (${TINT_BUILD_FUZZERS})
  # Tint library with fuzzer instrumentation
  add_library(libtint-fuzz ${TINT_LIB_SRCS})
  tint_default_compile_options(libtint-fuzz)
  if (${COMPILER_IS_LIKE_GNU})
    target_compile_options(libtint-fuzz PRIVATE -fvisibility=hidden)
  endif()

  if (NOT ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS} STREQUAL "")
    # This is set when the fuzzers are being built by OSS-Fuzz. In this case the
    # variable provides the necessary linker flags, and OSS-Fuzz will take care
    # of passing suitable compiler flags.
    target_link_options(libtint-fuzz PUBLIC ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS})
  else()
    # When the fuzzers are being built outside of OSS-Fuzz, specific libFuzzer
    # arguments to enable fuzzing are used.
    target_compile_options(libtint-fuzz PUBLIC -fsanitize=fuzzer -fsanitize-coverage=trace-cmp)
    target_link_options(libtint-fuzz PUBLIC -fsanitize=fuzzer -fsanitize-coverage=trace-cmp)
  endif()
endif()

if(${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
  tint_spvtools_compile_options(libtint)
  if (${TINT_BUILD_FUZZERS})
    tint_spvtools_compile_options(libtint-fuzz)
  endif()
endif()

if(${TINT_BUILD_TESTS})
  set(TINT_TEST_SRCS
    ast/alias_test.cc
    ast/array_test.cc
    ast/assignment_statement_test.cc
    ast/atomic_test.cc
    ast/binary_expression_test.cc
    ast/binding_decoration_test.cc
    ast/bitcast_expression_test.cc
    ast/block_statement_test.cc
    ast/bool_literal_expression_test.cc
    ast/bool_test.cc
    ast/break_statement_test.cc
    ast/builtin_decoration_test.cc
    ast/call_expression_test.cc
    ast/call_statement_test.cc
    ast/case_statement_test.cc
    ast/continue_statement_test.cc
    ast/depth_multisampled_texture_test.cc
    ast/depth_texture_test.cc
    ast/discard_statement_test.cc
    ast/else_statement_test.cc
    ast/external_texture_test.cc
    ast/f32_test.cc
    ast/fallthrough_statement_test.cc
    ast/float_literal_expression_test.cc
    ast/for_loop_statement_test.cc
    ast/function_test.cc
    ast/group_decoration_test.cc
    ast/i32_test.cc
    ast/identifier_expression_test.cc
    ast/if_statement_test.cc
    ast/index_accessor_expression_test.cc
    ast/int_literal_expression_test.cc
    ast/interpolate_decoration_test.cc
    ast/intrinsic_texture_helper_test.cc
    ast/intrinsic_texture_helper_test.h
    ast/invariant_decoration_test.cc
    ast/location_decoration_test.cc
    ast/loop_statement_test.cc
    ast/matrix_test.cc
    ast/member_accessor_expression_test.cc
    ast/module_clone_test.cc
    ast/module_test.cc
    ast/multisampled_texture_test.cc
    ast/override_decoration_test.cc
    ast/phony_expression_test.cc
    ast/pointer_test.cc
    ast/return_statement_test.cc
    ast/sampled_texture_test.cc
    ast/sampler_test.cc
    ast/sint_literal_expression_test.cc
    ast/stage_decoration_test.cc
    ast/storage_texture_test.cc
    ast/stride_decoration_test.cc
    ast/struct_member_align_decoration_test.cc
    ast/struct_member_offset_decoration_test.cc
    ast/struct_member_size_decoration_test.cc
    ast/struct_member_test.cc
    ast/struct_test.cc
    ast/switch_statement_test.cc
    ast/test_helper.h
    ast/texture_test.cc
    ast/traverse_expressions_test.cc
    ast/u32_test.cc
    ast/uint_literal_expression_test.cc
    ast/unary_op_expression_test.cc
    ast/variable_decl_statement_test.cc
    ast/variable_test.cc
    ast/vector_test.cc
    ast/workgroup_decoration_test.cc
    block_allocator_test.cc
    castable_test.cc
    clone_context_test.cc
    debug_test.cc
    demangler_test.cc
    diagnostic/formatter_test.cc
    diagnostic/printer_test.cc
    intrinsic_table_test.cc
    program_test.cc
    resolver/array_accessor_test.cc
    resolver/assignment_validation_test.cc
    resolver/atomics_test.cc
    resolver/atomics_validation_test.cc
    resolver/bitcast_validation_test.cc
    resolver/builtins_validation_test.cc
    resolver/call_test.cc
    resolver/call_validation_test.cc
    resolver/compound_statement_test.cc
    resolver/control_block_validation_test.cc
    resolver/decoration_validation_test.cc
    resolver/dependency_graph_test.cc
    resolver/entry_point_validation_test.cc
    resolver/function_validation_test.cc
    resolver/host_shareable_validation_test.cc
    resolver/inferred_type_test.cc
    resolver/intrinsic_test.cc
    resolver/intrinsic_validation_test.cc
    resolver/is_host_shareable_test.cc
    resolver/is_storeable_test.cc
    resolver/pipeline_overridable_constant_test.cc
    resolver/ptr_ref_test.cc
    resolver/ptr_ref_validation_test.cc
    resolver/resolver_behavior_test.cc
    resolver/resolver_constants_test.cc
    resolver/resolver_test_helper.cc
    resolver/resolver_test_helper.h
    resolver/resolver_test.cc
    resolver/storage_class_layout_validation_test.cc
    resolver/storage_class_validation_test.cc
    resolver/struct_layout_test.cc
    resolver/struct_pipeline_stage_use_test.cc
    resolver/struct_storage_class_use_test.cc
    resolver/type_constructor_validation_test.cc
    resolver/type_validation_test.cc
    resolver/validation_test.cc
    resolver/var_let_test.cc
    resolver/var_let_validation_test.cc
    scope_stack_test.cc
    sem/atomic_type_test.cc
    sem/bool_type_test.cc
    sem/depth_multisampled_texture_type_test.cc
    sem/depth_texture_type_test.cc
    sem/external_texture_type_test.cc
    sem/f32_type_test.cc
    sem/i32_type_test.cc
    sem/intrinsic_test.cc
    sem/matrix_type_test.cc
    sem/multisampled_texture_type_test.cc
    sem/pointer_type_test.cc
    sem/reference_type_test.cc
    sem/sampled_texture_type_test.cc
    sem/sampler_type_test.cc
    sem/sem_array_test.cc
    sem/sem_struct_test.cc
    sem/storage_texture_type_test.cc
    sem/texture_type_test.cc
    sem/type_manager_test.cc
    sem/u32_type_test.cc
    sem/vector_type_test.cc
    symbol_table_test.cc
    symbol_test.cc
    test_main.cc
    traits_test.cc
    transform/transform_test.cc
    utils/defer_test.cc
    utils/enum_set_test.cc
    utils/hash_test.cc
    utils/io/command_test.cc
    utils/io/tmpfile_test.cc
    utils/map_test.cc
    utils/math_test.cc
    utils/reverse_test.cc
    utils/scoped_assignment_test.cc
    utils/string_test.cc
    utils/transform_test.cc
    utils/unique_vector_test.cc
    writer/append_vector_test.cc
    writer/float_to_string_test.cc
    writer/text_generator_test.cc
  )

  # Inspector tests depend on WGSL reader
  if(${TINT_BUILD_WGSL_READER})
    list(APPEND TINT_TEST_SRCS
      inspector/inspector_test.cc
      inspector/test_inspector_builder.cc
      inspector/test_inspector_builder.h
      inspector/test_inspector_runner.cc
      inspector/test_inspector_runner.h
    )
  endif()

  if(${TINT_BUILD_SPV_READER} AND ${TINT_BUILD_WGSL_WRITER})
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
      reader/spirv/parser_impl_barrier_test.cc
      reader/spirv/parser_impl_convert_member_decoration_test.cc
      reader/spirv/parser_impl_convert_type_test.cc
      reader/spirv/parser_impl_function_decl_test.cc
      reader/spirv/parser_impl_get_decorations_test.cc
      reader/spirv/parser_impl_handle_test.cc
      reader/spirv/parser_impl_import_test.cc
      reader/spirv/parser_impl_module_var_test.cc
      reader/spirv/parser_impl_named_types_test.cc
      reader/spirv/parser_impl_test_helper.cc
      reader/spirv/parser_impl_test_helper.h
      reader/spirv/parser_impl_test.cc
      reader/spirv/parser_impl_user_name_test.cc
      reader/spirv/parser_type_test.cc
      reader/spirv/parser_test.cc
      reader/spirv/spirv_tools_helpers_test.cc
      reader/spirv/spirv_tools_helpers_test.h
      reader/spirv/usage_test.cc
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
      reader/wgsl/parser_impl_bug_cases_test.cc
      reader/wgsl/parser_impl_call_stmt_test.cc
      reader/wgsl/parser_impl_case_body_test.cc
      reader/wgsl/parser_impl_const_expr_test.cc
      reader/wgsl/parser_impl_const_literal_test.cc
      reader/wgsl/parser_impl_continue_stmt_test.cc
      reader/wgsl/parser_impl_continuing_stmt_test.cc
      reader/wgsl/parser_impl_depth_texture_type_test.cc
      reader/wgsl/parser_impl_external_texture_type_test.cc
      reader/wgsl/parser_impl_else_stmt_test.cc
      reader/wgsl/parser_impl_elseif_stmt_test.cc
      reader/wgsl/parser_impl_equality_expression_test.cc
      reader/wgsl/parser_impl_error_msg_test.cc
      reader/wgsl/parser_impl_error_resync_test.cc
      reader/wgsl/parser_impl_exclusive_or_expression_test.cc
      reader/wgsl/parser_impl_for_stmt_test.cc
      reader/wgsl/parser_impl_function_decl_test.cc
      reader/wgsl/parser_impl_function_decoration_list_test.cc
      reader/wgsl/parser_impl_function_decoration_test.cc
      reader/wgsl/parser_impl_function_header_test.cc
      reader/wgsl/parser_impl_global_constant_decl_test.cc
      reader/wgsl/parser_impl_global_decl_test.cc
      reader/wgsl/parser_impl_global_variable_decl_test.cc
      reader/wgsl/parser_impl_if_stmt_test.cc
      reader/wgsl/parser_impl_image_storage_type_test.cc
      reader/wgsl/parser_impl_inclusive_or_expression_test.cc
      reader/wgsl/parser_impl_logical_and_expression_test.cc
      reader/wgsl/parser_impl_logical_or_expression_test.cc
      reader/wgsl/parser_impl_loop_stmt_test.cc
      reader/wgsl/parser_impl_multiplicative_expression_test.cc
      reader/wgsl/parser_impl_param_list_test.cc
      reader/wgsl/parser_impl_paren_rhs_stmt_test.cc
      reader/wgsl/parser_impl_pipeline_stage_test.cc
      reader/wgsl/parser_impl_primary_expression_test.cc
      reader/wgsl/parser_impl_relational_expression_test.cc
      reader/wgsl/parser_impl_reserved_keyword_test.cc
      reader/wgsl/parser_impl_sampled_texture_type_test.cc
      reader/wgsl/parser_impl_sampler_type_test.cc
      reader/wgsl/parser_impl_shift_expression_test.cc
      reader/wgsl/parser_impl_singular_expression_test.cc
      reader/wgsl/parser_impl_statement_test.cc
      reader/wgsl/parser_impl_statements_test.cc
      reader/wgsl/parser_impl_storage_class_test.cc
      reader/wgsl/parser_impl_storage_texture_type_test.cc
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
      reader/wgsl/parser_impl_texture_sampler_types_test.cc
      reader/wgsl/parser_impl_type_alias_test.cc
      reader/wgsl/parser_impl_type_decl_test.cc
      reader/wgsl/parser_impl_unary_expression_test.cc
      reader/wgsl/parser_impl_variable_decl_test.cc
      reader/wgsl/parser_impl_variable_decoration_list_test.cc
      reader/wgsl/parser_impl_variable_decoration_test.cc
      reader/wgsl/parser_impl_variable_ident_decl_test.cc
      reader/wgsl/parser_impl_variable_stmt_test.cc
      reader/wgsl/parser_impl_variable_qualifier_test.cc
      reader/wgsl/token_test.cc
    )
  endif()

  if(${TINT_BUILD_SPV_WRITER})
    list(APPEND TINT_TEST_SRCS
      writer/spirv/binary_writer_test.cc
      writer/spirv/builder_accessor_expression_test.cc
      writer/spirv/builder_assign_test.cc
      writer/spirv/builder_binary_expression_test.cc
      writer/spirv/builder_bitcast_expression_test.cc
      writer/spirv/builder_block_test.cc
      writer/spirv/builder_call_test.cc
      writer/spirv/builder_constructor_expression_test.cc
      writer/spirv/builder_discard_test.cc
      writer/spirv/builder_entry_point_test.cc
      writer/spirv/builder_format_conversion_test.cc
      writer/spirv/builder_function_decoration_test.cc
      writer/spirv/builder_function_test.cc
      writer/spirv/builder_function_variable_test.cc
      writer/spirv/builder_global_variable_test.cc
      writer/spirv/builder_ident_expression_test.cc
      writer/spirv/builder_if_test.cc
      writer/spirv/builder_intrinsic_test.cc
      writer/spirv/builder_intrinsic_texture_test.cc
      writer/spirv/builder_literal_test.cc
      writer/spirv/builder_loop_test.cc
      writer/spirv/builder_return_test.cc
      writer/spirv/builder_switch_test.cc
      writer/spirv/builder_test.cc
      writer/spirv/builder_type_test.cc
      writer/spirv/builder_unary_op_expression_test.cc
      writer/spirv/instruction_test.cc
      writer/spirv/operand_test.cc
      writer/spirv/scalar_constant_test.cc
      writer/spirv/spv_dump.cc
      writer/spirv/spv_dump.h
      writer/spirv/test_helper.h
    )
  endif()

  if(${TINT_BUILD_WGSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      writer/wgsl/generator_impl_test.cc
      writer/wgsl/generator_impl_alias_type_test.cc
      writer/wgsl/generator_impl_array_accessor_test.cc
      writer/wgsl/generator_impl_assign_test.cc
      writer/wgsl/generator_impl_binary_test.cc
      writer/wgsl/generator_impl_bitcast_test.cc
      writer/wgsl/generator_impl_block_test.cc
      writer/wgsl/generator_impl_break_test.cc
      writer/wgsl/generator_impl_call_test.cc
      writer/wgsl/generator_impl_case_test.cc
      writer/wgsl/generator_impl_cast_test.cc
      writer/wgsl/generator_impl_constructor_test.cc
      writer/wgsl/generator_impl_continue_test.cc
      writer/wgsl/generator_impl_discard_test.cc
      writer/wgsl/generator_impl_fallthrough_test.cc
      writer/wgsl/generator_impl_function_test.cc
      writer/wgsl/generator_impl_global_decl_test.cc
      writer/wgsl/generator_impl_identifier_test.cc
      writer/wgsl/generator_impl_if_test.cc
      writer/wgsl/generator_impl_loop_test.cc
      writer/wgsl/generator_impl_literal_test.cc
      writer/wgsl/generator_impl_member_accessor_test.cc
      writer/wgsl/generator_impl_return_test.cc
      writer/wgsl/generator_impl_switch_test.cc
      writer/wgsl/generator_impl_type_test.cc
      writer/wgsl/generator_impl_unary_op_test.cc
      writer/wgsl/generator_impl_variable_decl_statement_test.cc
      writer/wgsl/generator_impl_variable_test.cc
      writer/wgsl/test_helper.h
    )
  endif()

  if(${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_WGSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      transform/add_empty_entry_point_test.cc
      transform/array_length_from_uniform_test.cc
      transform/binding_remapper_test.cc
      transform/calculate_array_length_test.cc
      transform/canonicalize_entry_point_io_test.cc
      transform/decompose_memory_access_test.cc
      transform/decompose_strided_matrix_test.cc
      transform/external_texture_transform_test.cc
      transform/first_index_offset_test.cc
      transform/fold_constants_test.cc
      transform/fold_trivial_single_use_lets_test.cc
      transform/for_loop_to_loop_test.cc
      transform/loop_to_for_loop_test.cc
      transform/module_scope_var_to_entry_point_param_test.cc
      transform/multiplanar_external_texture_test.cc
      transform/num_workgroups_from_uniform_test.cc
      transform/pad_array_elements_test.cc
      transform/promote_initializers_to_const_var_test.cc
      transform/remove_phonies_test.cc
      transform/remove_unreachable_statements_test.cc
      transform/renamer_test.cc
      transform/robustness_test.cc
      transform/simplify_pointers_test.cc
      transform/single_entry_point_test.cc
      transform/test_helper.h
      transform/unshadow_test.cc
      transform/vectorize_scalar_matrix_constructors_test.cc
      transform/vertex_pulling_test.cc
      transform/wrap_arrays_in_structs_test.cc
      transform/zero_init_workgroup_memory_test.cc
    )
  endif()

  if(${TINT_BUILD_MSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      writer/msl/generator_impl_array_accessor_test.cc
      writer/msl/generator_impl_assign_test.cc
      writer/msl/generator_impl_binary_test.cc
      writer/msl/generator_impl_bitcast_test.cc
      writer/msl/generator_impl_block_test.cc
      writer/msl/generator_impl_break_test.cc
      writer/msl/generator_impl_call_test.cc
      writer/msl/generator_impl_case_test.cc
      writer/msl/generator_impl_cast_test.cc
      writer/msl/generator_impl_constructor_test.cc
      writer/msl/generator_impl_continue_test.cc
      writer/msl/generator_impl_discard_test.cc
      writer/msl/generator_impl_function_test.cc
      writer/msl/generator_impl_identifier_test.cc
      writer/msl/generator_impl_if_test.cc
      writer/msl/generator_impl_import_test.cc
      writer/msl/generator_impl_intrinsic_test.cc
      writer/msl/generator_impl_intrinsic_texture_test.cc
      writer/msl/generator_impl_loop_test.cc
      writer/msl/generator_impl_member_accessor_test.cc
      writer/msl/generator_impl_module_constant_test.cc
      writer/msl/generator_impl_return_test.cc
      writer/msl/generator_impl_sanitizer_test.cc
      writer/msl/generator_impl_switch_test.cc
      writer/msl/generator_impl_test.cc
      writer/msl/generator_impl_type_test.cc
      writer/msl/generator_impl_unary_op_test.cc
      writer/msl/generator_impl_variable_decl_statement_test.cc
      writer/msl/test_helper.h
    )
  endif()

  if (${TINT_BUILD_GLSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      writer/glsl/generator_impl_array_accessor_test.cc
      writer/glsl/generator_impl_assign_test.cc
      writer/glsl/generator_impl_binary_test.cc
      writer/glsl/generator_impl_bitcast_test.cc
      writer/glsl/generator_impl_block_test.cc
      writer/glsl/generator_impl_break_test.cc
      writer/glsl/generator_impl_call_test.cc
      writer/glsl/generator_impl_case_test.cc
      writer/glsl/generator_impl_cast_test.cc
      writer/glsl/generator_impl_constructor_test.cc
      writer/glsl/generator_impl_continue_test.cc
      writer/glsl/generator_impl_discard_test.cc
      writer/glsl/generator_impl_function_test.cc
      writer/glsl/generator_impl_identifier_test.cc
      writer/glsl/generator_impl_if_test.cc
      writer/glsl/generator_impl_intrinsic_test.cc
      writer/glsl/generator_impl_intrinsic_texture_test.cc
      writer/glsl/generator_impl_import_test.cc
      writer/glsl/generator_impl_loop_test.cc
      writer/glsl/generator_impl_member_accessor_test.cc
      writer/glsl/generator_impl_module_constant_test.cc
      writer/glsl/generator_impl_return_test.cc
      writer/glsl/generator_impl_sanitizer_test.cc
      writer/glsl/generator_impl_switch_test.cc
      writer/glsl/generator_impl_test.cc
      writer/glsl/generator_impl_type_test.cc
      writer/glsl/generator_impl_unary_op_test.cc
      writer/glsl/generator_impl_variable_decl_statement_test.cc
      writer/glsl/generator_impl_workgroup_var_test.cc
      writer/glsl/test_helper.h
    )
  endif()

  if (${TINT_BUILD_HLSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      writer/hlsl/generator_impl_array_accessor_test.cc
      writer/hlsl/generator_impl_assign_test.cc
      writer/hlsl/generator_impl_binary_test.cc
      writer/hlsl/generator_impl_bitcast_test.cc
      writer/hlsl/generator_impl_block_test.cc
      writer/hlsl/generator_impl_break_test.cc
      writer/hlsl/generator_impl_call_test.cc
      writer/hlsl/generator_impl_case_test.cc
      writer/hlsl/generator_impl_cast_test.cc
      writer/hlsl/generator_impl_constructor_test.cc
      writer/hlsl/generator_impl_continue_test.cc
      writer/hlsl/generator_impl_discard_test.cc
      writer/hlsl/generator_impl_function_test.cc
      writer/hlsl/generator_impl_identifier_test.cc
      writer/hlsl/generator_impl_if_test.cc
      writer/hlsl/generator_impl_intrinsic_test.cc
      writer/hlsl/generator_impl_intrinsic_texture_test.cc
      writer/hlsl/generator_impl_import_test.cc
      writer/hlsl/generator_impl_loop_test.cc
      writer/hlsl/generator_impl_member_accessor_test.cc
      writer/hlsl/generator_impl_module_constant_test.cc
      writer/hlsl/generator_impl_return_test.cc
      writer/hlsl/generator_impl_sanitizer_test.cc
      writer/hlsl/generator_impl_switch_test.cc
      writer/hlsl/generator_impl_test.cc
      writer/hlsl/generator_impl_type_test.cc
      writer/hlsl/generator_impl_unary_op_test.cc
      writer/hlsl/generator_impl_variable_decl_statement_test.cc
      writer/hlsl/generator_impl_workgroup_var_test.cc
      writer/hlsl/test_helper.h
    )
  endif()

  if (${TINT_BUILD_FUZZERS})
    list(APPEND TINT_TEST_SRCS
      ../fuzzers/mersenne_twister_engine.cc
      ../fuzzers/mersenne_twister_engine.h
      ../fuzzers/random_generator.cc
      ../fuzzers/random_generator.h
      ../fuzzers/random_generator_engine.cc
      ../fuzzers/random_generator_engine.h
      ../fuzzers/random_generator_test.cc
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
  target_link_libraries(tint_unittests libtint gmock tint_utils_io)
  tint_default_compile_options(tint_unittests)

  if(${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
    tint_spvtools_compile_options(tint_unittests)
  endif()

  add_test(NAME tint_unittests COMMAND tint_unittests)
endif()
