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
endfunction()

set(TINT_LIB_SRCS)
set(TINT_BENCHMARK_SRCS)
set(TINT_TEST_SRCS)

# Add a generated file set into the build.
#
# Params:
#  TARGET - the target name to add (without extension)
#  BENCH  - set if the target has a benchmark file
#  TEST   - set if the target has a test file
#
function(tint_generated TARGET)
  cmake_parse_arguments(PARSE_ARGV 0 TINT_GEN "BENCH;TEST" "" "")

   list(APPEND TINT_LIB_SRCS
      ${TARGET}.cc
      ${TARGET}.h
    )
    set(TINT_LIB_SRCS ${TINT_LIB_SRCS} PARENT_SCOPE)

    if(${TINT_GEN_BENCH})
        list(APPEND TINT_BENCHMARK_SRCS ${TARGET}_bench.cc)
        set(TINT_BENCHMARK_SRCS ${TINT_BENCHMARK_SRCS} PARENT_SCOPE)
    endif()
    if(${TINT_GEN_TEST})
        list(APPEND TINT_TEST_SRCS ${TARGET}_test.cc)
        set(TINT_TEST_SRCS ${TINT_TEST_SRCS} PARENT_SCOPE)
    endif()
endfunction()

## Tint diagnostic utilities. Used by libtint and tint_utils_io.
add_library(tint_diagnostic_utils
  debug.cc
  debug.h
  source.h
  source.cc
  diagnostic/diagnostic.cc
  diagnostic/diagnostic.h
  diagnostic/formatter.cc
  diagnostic/formatter.h
  diagnostic/printer.cc
  diagnostic/printer.h
  utils/debugger.cc
  utils/debugger.h
  utils/unicode.cc
  utils/unicode.h
)
tint_default_compile_options(tint_diagnostic_utils)

if (TINT_ENABLE_BREAK_IN_DEBUGGER)
  set_source_files_properties(utils/debugger.cc
    PROPERTIES COMPILE_DEFINITIONS "TINT_ENABLE_BREAK_IN_DEBUGGER=1" )
endif()

list(APPEND TINT_LIB_SRCS
  ../../include/tint/tint.h
  ast/accessor_expression.cc
  ast/accessor_expression.h
  ast/alias.cc
  ast/alias.h
  ast/assignment_statement.cc
  ast/assignment_statement.h
  ast/attribute.cc
  ast/attribute.h
  ast/binary_expression.cc
  ast/binary_expression.h
  ast/binding_attribute.cc
  ast/binding_attribute.h
  ast/bitcast_expression.cc
  ast/bitcast_expression.h
  ast/block_statement.cc
  ast/block_statement.h
  ast/bool_literal_expression.cc
  ast/bool_literal_expression.h
  ast/break_if_statement.cc
  ast/break_if_statement.h
  ast/break_statement.cc
  ast/break_statement.h
  ast/builtin_attribute.cc
  ast/builtin_attribute.h
  ast/call_expression.cc
  ast/call_expression.h
  ast/call_statement.cc
  ast/call_statement.h
  ast/case_selector.cc
  ast/case_selector.h
  ast/case_statement.cc
  ast/case_statement.h
  ast/compound_assignment_statement.cc
  ast/compound_assignment_statement.h
  ast/const.cc
  ast/const.h
  ast/const_assert.cc
  ast/const_assert.h
  ast/continue_statement.cc
  ast/continue_statement.h
  ast/diagnostic_attribute.cc
  ast/diagnostic_attribute.h
  ast/diagnostic_control.cc
  ast/diagnostic_control.h
  ast/diagnostic_directive.cc
  ast/diagnostic_directive.h
  ast/diagnostic_rule_name.cc
  ast/diagnostic_rule_name.h
  ast/disable_validation_attribute.cc
  ast/disable_validation_attribute.h
  ast/discard_statement.cc
  ast/discard_statement.h
  ast/enable.cc
  ast/enable.h
  ast/expression.cc
  ast/expression.h
  ast/extension.cc
  ast/extension.h
  ast/float_literal_expression.cc
  ast/float_literal_expression.h
  ast/for_loop_statement.cc
  ast/for_loop_statement.h
  ast/function.cc
  ast/function.h
  ast/group_attribute.cc
  ast/group_attribute.h
  ast/id_attribute.cc
  ast/id_attribute.h
  ast/identifier.cc
  ast/identifier.h
  ast/identifier_expression.cc
  ast/identifier_expression.h
  ast/if_statement.cc
  ast/if_statement.h
  ast/increment_decrement_statement.cc
  ast/increment_decrement_statement.h
  ast/index_accessor_expression.cc
  ast/index_accessor_expression.h
  ast/int_literal_expression.cc
  ast/int_literal_expression.h
  ast/internal_attribute.cc
  ast/internal_attribute.h
  ast/interpolate_attribute.cc
  ast/interpolate_attribute.h
  ast/invariant_attribute.cc
  ast/invariant_attribute.h
  ast/let.cc
  ast/let.h
  ast/literal_expression.cc
  ast/literal_expression.h
  ast/location_attribute.cc
  ast/location_attribute.h
  ast/loop_statement.cc
  ast/loop_statement.h
  ast/member_accessor_expression.cc
  ast/member_accessor_expression.h
  ast/module.cc
  ast/module.h
  ast/must_use_attribute.cc
  ast/must_use_attribute.h
  ast/node_id.h
  ast/node.cc
  ast/node.h
  ast/override.cc
  ast/override.h
  ast/parameter.cc
  ast/parameter.h
  ast/phony_expression.cc
  ast/phony_expression.h
  ast/pipeline_stage.cc
  ast/pipeline_stage.h
  ast/return_statement.cc
  ast/return_statement.h
  ast/stage_attribute.cc
  ast/stage_attribute.h
  ast/statement.cc
  ast/statement.h
  ast/stride_attribute.cc
  ast/stride_attribute.h
  ast/struct_member_align_attribute.cc
  ast/struct_member_align_attribute.h
  ast/struct_member_offset_attribute.cc
  ast/struct_member_offset_attribute.h
  ast/struct_member_size_attribute.cc
  ast/struct_member_size_attribute.h
  ast/struct_member.cc
  ast/struct_member.h
  ast/struct.cc
  ast/struct.h
  ast/switch_statement.cc
  ast/switch_statement.h
  ast/templated_identifier.cc
  ast/templated_identifier.h
  ast/traverse_expressions.h
  ast/type.cc
  ast/type.h
  ast/type_decl.cc
  ast/type_decl.h
  ast/unary_op_expression.cc
  ast/unary_op_expression.h
  ast/unary_op.cc
  ast/unary_op.h
  ast/var.cc
  ast/var.h
  ast/variable_decl_statement.cc
  ast/variable_decl_statement.h
  ast/variable.cc
  ast/variable.h
  ast/while_statement.cc
  ast/while_statement.h
  ast/workgroup_attribute.cc
  ast/workgroup_attribute.h
  clone_context.cc
  clone_context.h
  constant/clone_context.h
  constant/composite.cc
  constant/composite.h
  constant/scalar.cc
  constant/scalar.h
  constant/splat.cc
  constant/splat.h
  constant/node.cc
  constant/node.h
  constant/value.cc
  constant/value.h
  inspector/entry_point.cc
  inspector/entry_point.h
  inspector/inspector.cc
  inspector/inspector.h
  inspector/resource_binding.cc
  inspector/resource_binding.h
  inspector/scalar.cc
  inspector/scalar.h
  number.cc
  number.h
  program_builder.cc
  program_builder.h
  program_id.cc
  program_id.h
  program.cc
  program.h
  reflection.h
  reader/reader.cc
  reader/reader.h
  resolver/builtin_structs.cc
  resolver/builtin_structs.h
  resolver/const_eval.cc
  resolver/const_eval.h
  resolver/dependency_graph.cc
  resolver/dependency_graph.h
  resolver/intrinsic_table.cc
  resolver/intrinsic_table.h
  resolver/intrinsic_table.inl
  resolver/resolver.cc
  resolver/resolver.h
  resolver/sem_helper.cc
  resolver/sem_helper.h
  resolver/uniformity.cc
  resolver/uniformity.h
  resolver/validator.cc
  resolver/validator.h
  scope_stack.h
  switch.h
  sem/array_count.cc
  sem/array_count.h
  sem/behavior.cc
  sem/behavior.h
  sem/binding_point.h
  sem/block_statement.cc
  sem/block_statement.h
  sem/break_if_statement.cc
  sem/break_if_statement.h
  sem/builtin.cc
  sem/builtin.h
  sem/builtin_enum_expression.cc
  sem/builtin_enum_expression.h
  sem/call_target.cc
  sem/call_target.h
  sem/call.cc
  sem/call.h
  sem/evaluation_stage.h
  sem/expression.cc
  sem/expression.h
  sem/external_texture.h
  sem/for_loop_statement.cc
  sem/for_loop_statement.h
  sem/function_expression.cc
  sem/function_expression.h
  sem/function.cc
  sem/if_statement.cc
  sem/if_statement.h
  sem/index_accessor_expression.cc
  sem/index_accessor_expression.h
  sem/info.cc
  sem/info.h
  sem/load.cc
  sem/load.h
  sem/loop_statement.cc
  sem/loop_statement.h
  sem/materialize.cc
  sem/materialize.h
  sem/member_accessor_expression.cc
  sem/module.cc
  sem/module.h
  sem/node.cc
  sem/node.h
  sem/pipeline_stage_set.h
  sem/sampler_texture_pair.h
  sem/statement.cc
  sem/struct.cc
  sem/struct.h
  sem/switch_statement.cc
  sem/switch_statement.h
  sem/type_expression.cc
  sem/type_expression.h
  sem/type_mappings.h
  sem/variable.cc
  sem/value_constructor.cc
  sem/value_constructor.h
  sem/value_conversion.cc
  sem/value_conversion.h
  sem/value_expression.cc
  sem/value_expression.h
  sem/while_statement.cc
  sem/while_statement.h
  symbol_table.cc
  symbol_table.h
  symbol.cc
  symbol.h
  tint.cc
  transform/add_empty_entry_point.cc
  transform/add_empty_entry_point.h
  transform/add_block_attribute.cc
  transform/add_block_attribute.h
  transform/array_length_from_uniform.cc
  transform/array_length_from_uniform.h
  transform/binding_remapper.cc
  transform/binding_remapper.h
  transform/builtin_polyfill.cc
  transform/builtin_polyfill.h
  transform/calculate_array_length.cc
  transform/calculate_array_length.h
  transform/clamp_frag_depth.cc
  transform/clamp_frag_depth.h
  transform/canonicalize_entry_point_io.cc
  transform/canonicalize_entry_point_io.h
  transform/combine_samplers.cc
  transform/combine_samplers.h
  transform/decompose_memory_access.cc
  transform/decompose_memory_access.h
  transform/decompose_strided_array.cc
  transform/decompose_strided_array.h
  transform/decompose_strided_matrix.cc
  transform/decompose_strided_matrix.h
  transform/demote_to_helper.cc
  transform/demote_to_helper.h
  transform/direct_variable_access.cc
  transform/direct_variable_access.h
  transform/disable_uniformity_analysis.cc
  transform/disable_uniformity_analysis.h
  transform/expand_compound_assignment.cc
  transform/expand_compound_assignment.h
  transform/first_index_offset.cc
  transform/first_index_offset.h
  transform/for_loop_to_loop.cc
  transform/for_loop_to_loop.h
  transform/localize_struct_array_assignment.cc
  transform/localize_struct_array_assignment.h
  transform/manager.cc
  transform/manager.h
  transform/merge_return.cc
  transform/merge_return.h
  transform/module_scope_var_to_entry_point_param.cc
  transform/module_scope_var_to_entry_point_param.h
  transform/multiplanar_external_texture.cc
  transform/multiplanar_external_texture.h
  transform/num_workgroups_from_uniform.cc
  transform/num_workgroups_from_uniform.h
  transform/packed_vec3.cc
  transform/packed_vec3.h
  transform/pad_structs.cc
  transform/pad_structs.h
  transform/preserve_padding.cc
  transform/preserve_padding.h
  transform/promote_initializers_to_let.cc
  transform/promote_initializers_to_let.h
  transform/promote_side_effects_to_decl.cc
  transform/promote_side_effects_to_decl.h
  transform/remove_continue_in_switch.cc
  transform/remove_continue_in_switch.h
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
  transform/spirv_atomic.cc
  transform/spirv_atomic.h
  transform/std140.cc
  transform/std140.h
  transform/substitute_override.cc
  transform/substitute_override.h
  transform/texture_1d_to_2d.cc
  transform/texture_1d_to_2d.h
  transform/transform.cc
  transform/transform.h
  transform/truncate_interstage_variables.cc
  transform/truncate_interstage_variables.h
  transform/unshadow.cc
  transform/unshadow.h
  transform/utils/get_insertion_point.cc
  transform/utils/get_insertion_point.h
  transform/utils/hoist_to_decl_before.cc
  transform/utils/hoist_to_decl_before.h
  transform/var_for_dynamic_index.cc
  transform/var_for_dynamic_index.h
  transform/vectorize_matrix_conversions.cc
  transform/vectorize_matrix_conversions.h
  transform/vectorize_scalar_matrix_initializers.cc
  transform/vectorize_scalar_matrix_initializers.h
  transform/vertex_pulling.cc
  transform/vertex_pulling.h
  transform/while_to_loop.cc
  transform/while_to_loop.h
  transform/zero_init_workgroup_memory.cc
  transform/zero_init_workgroup_memory.h
  type/abstract_float.cc
  type/abstract_float.h
  type/abstract_int.cc
  type/abstract_int.h
  type/abstract_numeric.cc
  type/abstract_numeric.h
  type/array.cc
  type/array.h
  type/array_count.cc
  type/array_count.h
  type/atomic.cc
  type/atomic.h
  type/bool.cc
  type/bool.h
  type/clone_context.h
  type/depth_multisampled_texture.cc
  type/depth_multisampled_texture.h
  type/depth_texture.cc
  type/depth_texture.h
  type/external_texture.cc
  type/external_texture.h
  type/f16.cc
  type/f16.h
  type/f32.cc
  type/f32.h
  type/i32.cc
  type/i32.h
  type/manager.cc
  type/manager.h
  type/matrix.cc
  type/matrix.h
  type/multisampled_texture.cc
  type/multisampled_texture.h
  type/node.cc
  type/node.h
  type/pointer.cc
  type/pointer.h
  type/reference.cc
  type/reference.h
  type/sampled_texture.cc
  type/sampled_texture.h
  type/sampler.cc
  type/sampler.h
  type/sampler_kind.cc
  type/sampler_kind.h
  type/storage_texture.cc
  type/storage_texture.h
  type/struct.cc
  type/struct.h
  type/texture.cc
  type/texture.h
  type/texture_dimension.cc
  type/texture_dimension.h
  type/type.cc
  type/type.h
  type/u32.cc
  type/u32.h
  type/unique_node.cc
  type/unique_node.h
  type/vector.cc
  type/vector.h
  type/void.cc
  type/void.h
  utils/bitcast.h
  utils/bitset.h
  utils/block_allocator.h
  utils/bump_allocator.h
  utils/castable.cc
  utils/castable.h
  utils/compiler_macros.h
  utils/concat.h
  utils/crc32.h
  utils/enum_set.h
  utils/foreach_macro.h
  utils/hash.h
  utils/hashmap_base.h
  utils/hashmap.h
  utils/hashset.h
  utils/map.h
  utils/math.h
  utils/predicates.h
  utils/scoped_assignment.h
  utils/slice.h
  utils/string.cc
  utils/string.h
  utils/string_stream.cc
  utils/string_stream.h
  utils/traits.h
  utils/unique_allocator.h
  utils/unique_vector.h
  utils/vector.h
  writer/append_vector.cc
  writer/append_vector.h
  writer/array_length_from_uniform_options.cc
  writer/array_length_from_uniform_options.h
  writer/binding_point.h
  writer/binding_remapper_options.cc
  writer/binding_remapper_options.h
  writer/check_supported_extensions.cc
  writer/check_supported_extensions.h
  writer/external_texture_options.cc
  writer/external_texture_options.h
  writer/flatten_bindings.cc
  writer/flatten_bindings.h
  writer/float_to_string.cc
  writer/float_to_string.h
  writer/text_generator.cc
  writer/text_generator.h
  writer/text.cc
  writer/text.h
  writer/writer.cc
  writer/writer.h
)

tint_generated(builtin/access BENCH TEST)
tint_generated(builtin/address_space BENCH TEST)
tint_generated(builtin/attribute BENCH TEST)
tint_generated(builtin/builtin BENCH TEST)
tint_generated(builtin/builtin_value BENCH TEST)
tint_generated(builtin/diagnostic_rule BENCH TEST)
tint_generated(builtin/diagnostic_severity BENCH TEST)
tint_generated(builtin/extension BENCH TEST)
tint_generated(builtin/function)
tint_generated(builtin/interpolation_sampling BENCH TEST)
tint_generated(builtin/interpolation_type BENCH TEST)
tint_generated(builtin/texel_format BENCH TEST)

tint_generated(resolver/ctor_conv_intrinsic)
tint_generated(sem/parameter_usage)

if(UNIX)
  list(APPEND TINT_LIB_SRCS diagnostic/printer_posix.cc)
elseif(WIN32)
  list(APPEND TINT_LIB_SRCS diagnostic/printer_windows.cc)
else()
  list(APPEND TINT_LIB_SRCS diagnostic/printer_other.cc)
endif()

if(${TINT_BUILD_SPV_READER})
  list(APPEND TINT_LIB_SRCS
    reader/spirv/attributes.h
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
    reader/wgsl/classify_template_args.cc
    reader/wgsl/classify_template_args.h
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
    writer/spirv/generator_impl.cc
    writer/spirv/generator_impl.h
    writer/spirv/instruction.cc
    writer/spirv/instruction.h
    writer/spirv/module.cc
    writer/spirv/module.h
    writer/spirv/operand.cc
    writer/spirv/operand.h
    writer/spirv/scalar_constant.h
  )

  if(${TINT_BUILD_IR})
    list(APPEND TINT_LIB_SRCS
      writer/spirv/generator_impl_ir.cc
      writer/spirv/generator_impl_ir.h
    )
  endif()
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
    writer/glsl/version.h
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

if(${TINT_BUILD_SYNTAX_TREE_WRITER})
    list(APPEND TINT_LIB_SRCS
      writer/syntax_tree/generator.cc
      writer/syntax_tree/generator.h
      writer/syntax_tree/generator_impl.cc
      writer/syntax_tree/generator_impl.h
    )
endif()

if(${TINT_BUILD_IR})
  list(APPEND TINT_LIB_SRCS
    ir/binary.cc
    ir/binary.h
    ir/bitcast.cc
    ir/bitcast.h
    ir/block.cc
    ir/block.h
    ir/builder.cc
    ir/builder.h
    ir/builder_impl.cc
    ir/builder_impl.h
    ir/builtin.cc
    ir/builtin.h
    ir/call.cc
    ir/call.h
    ir/constant.cc
    ir/constant.h
    ir/construct.cc
    ir/construct.h
    ir/convert.cc
    ir/convert.h
    ir/debug.cc
    ir/debug.h
    ir/disassembler.cc
    ir/disassembler.h
    ir/discard.cc
    ir/discard.h
    ir/from_program.cc
    ir/from_program.h
    ir/flow_node.cc
    ir/flow_node.h
    ir/function.cc
    ir/function.h
    ir/function_terminator.cc
    ir/function_terminator.h
    ir/if.cc
    ir/if.h
    ir/instruction.cc
    ir/instruction.h
    ir/loop.cc
    ir/loop.h
    ir/module.cc
    ir/module.h
    ir/root_terminator.cc
    ir/root_terminator.h
    ir/store.cc
    ir/store.h
    ir/switch.cc
    ir/switch.h
    ir/unary.cc
    ir/unary.h
    ir/user_call.cc
    ir/user_call.h
    ir/value.cc
    ir/value.h
    ir/var.cc
    ir/var.h
  )
endif()

if(MSVC)
  list(APPEND TINT_LIB_SRCS
    tint.natvis
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
target_link_libraries(tint_utils_io tint_diagnostic_utils)

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
target_link_libraries(libtint tint_diagnostic_utils absl_strings)
set_target_properties(libtint PROPERTIES OUTPUT_NAME "tint")

if (${TINT_BUILD_FUZZERS})
  # Tint library with fuzzer instrumentation
  add_library(libtint-fuzz ${TINT_LIB_SRCS})
  tint_default_compile_options(libtint-fuzz)
  target_link_libraries(libtint-fuzz tint_diagnostic_utils absl_strings)
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


################################################################################
# Tests
################################################################################
if(TINT_BUILD_TESTS)
  list(APPEND TINT_TEST_SRCS
    ast/alias_test.cc
    ast/assignment_statement_test.cc
    ast/binary_expression_test.cc
    ast/binding_attribute_test.cc
    ast/bitcast_expression_test.cc
    ast/block_statement_test.cc
    ast/bool_literal_expression_test.cc
    ast/break_if_statement_test.cc
    ast/break_statement_test.cc
    ast/builtin_attribute_test.cc
    ast/builtin_texture_helper_test.cc
    ast/builtin_texture_helper_test.h
    ast/call_expression_test.cc
    ast/call_statement_test.cc
    ast/case_selector_test.cc
    ast/case_statement_test.cc
    ast/compound_assignment_statement_test.cc
    ast/const_assert_test.cc
    ast/continue_statement_test.cc
    ast/diagnostic_attribute_test.cc
    ast/diagnostic_control_test.cc
    ast/diagnostic_directive_test.cc
    ast/diagnostic_rule_name_test.cc
    ast/discard_statement_test.cc
    ast/enable_test.cc
    ast/float_literal_expression_test.cc
    ast/for_loop_statement_test.cc
    ast/function_test.cc
    ast/group_attribute_test.cc
    ast/id_attribute_test.cc
    ast/identifier_test.cc
    ast/identifier_expression_test.cc
    ast/if_statement_test.cc
    ast/increment_decrement_statement_test.cc
    ast/index_accessor_expression_test.cc
    ast/int_literal_expression_test.cc
    ast/interpolate_attribute_test.cc
    ast/location_attribute_test.cc
    ast/loop_statement_test.cc
    ast/member_accessor_expression_test.cc
    ast/module_test.cc
    ast/phony_expression_test.cc
    ast/return_statement_test.cc
    ast/stage_attribute_test.cc
    ast/stride_attribute_test.cc
    ast/struct_member_align_attribute_test.cc
    ast/struct_member_offset_attribute_test.cc
    ast/struct_member_size_attribute_test.cc
    ast/struct_member_test.cc
    ast/struct_test.cc
    ast/switch_statement_test.cc
    ast/templated_identifier_test.cc
    ast/test_helper.h
    ast/test_helper_test.cc
    ast/traverse_expressions_test.cc
    ast/unary_op_expression_test.cc
    ast/variable_decl_statement_test.cc
    ast/variable_test.cc
    ast/while_statement_test.cc
    ast/workgroup_attribute_test.cc
    clone_context_test.cc
    constant/composite_test.cc
    constant/scalar_test.cc
    constant/splat_test.cc
    debug_test.cc
    diagnostic/diagnostic_test.cc
    diagnostic/formatter_test.cc
    diagnostic/printer_test.cc
    number_test.cc
    program_builder_test.cc
    program_test.cc
    reflection_test.cc
    resolver/alias_analysis_test.cc
    resolver/array_accessor_test.cc
    resolver/assignment_validation_test.cc
    resolver/atomics_test.cc
    resolver/atomics_validation_test.cc
    resolver/attribute_validation_test.cc
    resolver/bitcast_validation_test.cc
    resolver/builtin_enum_test.cc
    resolver/builtin_structs_test.cc
    resolver/builtin_test.cc
    resolver/builtin_validation_test.cc
    resolver/builtins_validation_test.cc
    resolver/call_test.cc
    resolver/call_validation_test.cc
    resolver/compound_assignment_validation_test.cc
    resolver/compound_statement_test.cc
    resolver/const_assert_test.cc
    resolver/const_eval_binary_op_test.cc
    resolver/const_eval_bitcast_test.cc
    resolver/const_eval_builtin_test.cc
    resolver/const_eval_construction_test.cc
    resolver/const_eval_conversion_test.cc
    resolver/const_eval_indexing_test.cc
    resolver/const_eval_member_access_test.cc
    resolver/const_eval_runtime_semantics_test.cc
    resolver/const_eval_test.h
    resolver/const_eval_unary_op_test.cc
    resolver/control_block_validation_test.cc
    resolver/dependency_graph_test.cc
    resolver/diagnostic_control_test.cc
    resolver/entry_point_validation_test.cc
    resolver/evaluation_stage_test.cc
    resolver/expression_kind_test.cc
    resolver/f16_extension_test.cc
    resolver/function_validation_test.cc
    resolver/host_shareable_validation_test.cc
    resolver/increment_decrement_validation_test.cc
    resolver/inferred_type_test.cc
    resolver/intrinsic_table_test.cc
    resolver/is_host_shareable_test.cc
    resolver/is_storeable_test.cc
    resolver/load_test.cc
    resolver/materialize_test.cc
    resolver/override_test.cc
    resolver/ptr_ref_test.cc
    resolver/ptr_ref_validation_test.cc
    resolver/resolver_behavior_test.cc
    resolver/resolver_test_helper.cc
    resolver/resolver_test_helper.h
    resolver/resolver_test.cc
    resolver/side_effects_test.cc
    resolver/root_identifier_test.cc
    resolver/address_space_layout_validation_test.cc
    resolver/address_space_validation_test.cc
    resolver/struct_layout_test.cc
    resolver/struct_pipeline_stage_use_test.cc
    resolver/struct_address_space_use_test.cc
    resolver/type_validation_test.cc
    resolver/unresolved_identifier_test.cc
    resolver/validation_test.cc
    resolver/validator_is_storeable_test.cc
    resolver/value_constructor_validation_test.cc
    resolver/variable_test.cc
    resolver/variable_validation_test.cc
    scope_stack_test.cc
    sem/builtin_test.cc
    sem/diagnostic_severity_test.cc
    sem/struct_test.cc
    sem/value_expression_test.cc
    source_test.cc
    switch_test.cc
    symbol_table_test.cc
    symbol_test.cc
    test_main.cc
    transform/transform_test.cc
    type/array_test.cc
    type/atomic_test.cc
    type/bool_test.cc
    type/depth_multisampled_texture_test.cc
    type/depth_texture_test.cc
    type/external_texture_test.cc
    type/f16_test.cc
    type/f32_test.cc
    type/i32_test.cc
    type/manager_test.cc
    type/matrix_test.cc
    type/multisampled_texture_test.cc
    type/pointer_test.cc
    type/reference_test.cc
    type/sampled_texture_test.cc
    type/sampler_test.cc
    type/storage_texture_test.cc
    type/struct_test.cc
    type/texture_test.cc
    type/type_test.cc
    type/u32_test.cc
    type/vector_test.cc
    utils/bitcast_test.cc
    utils/bitset_test.cc
    utils/block_allocator_test.cc
    utils/bump_allocator_test.cc
    utils/castable_test.cc
    utils/crc32_test.cc
    utils/defer_test.cc
    utils/enum_set_test.cc
    utils/hash_test.cc
    utils/io/command_test.cc
    utils/io/tmpfile_test.cc
    utils/hashmap_test.cc
    utils/hashset_test.cc
    utils/map_test.cc
    utils/math_test.cc
    utils/predicates_test.cc
    utils/result_test.cc
    utils/reverse_test.cc
    utils/scoped_assignment_test.cc
    utils/slice_test.cc
    utils/string_stream_test.cc
    utils/string_test.cc
    utils/traits_test.cc
    utils/transform_test.cc
    utils/unicode_test.cc
    utils/unique_allocator_test.cc
    utils/unique_vector_test.cc
    utils/vector_test.cc
    writer/append_vector_test.cc
    writer/check_supported_extensions_test.cc
    writer/flatten_bindings_test.cc
    writer/float_to_string_test.cc
    writer/text_generator_test.cc
  )

  # Noet, the source files are included here otherwise the cmd sources would not be included in the
  # test binary.
  list(APPEND TINT_TEST_SRCS
    cmd/generate_external_texture_bindings.cc
    cmd/generate_external_texture_bindings.h
    cmd/generate_external_texture_bindings_test.cc
  )

  # Uniformity analysis tests depend on WGSL reader
  if(${TINT_BUILD_WGSL_READER})
    list(APPEND TINT_TEST_SRCS
      resolver/uniformity_test.cc
    )
  endif()

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
      reader/spirv/parser_impl_constant_test.cc
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
      reader/wgsl/classify_template_args_test.cc
      reader/wgsl/lexer_test.cc
      reader/wgsl/parser_test.cc
      reader/wgsl/parser_impl_additive_expression_test.cc
      reader/wgsl/parser_impl_argument_expression_list_test.cc
      reader/wgsl/parser_impl_assignment_stmt_test.cc
      reader/wgsl/parser_impl_bitwise_expression_test.cc
      reader/wgsl/parser_impl_break_stmt_test.cc
      reader/wgsl/parser_impl_bug_cases_test.cc
      reader/wgsl/parser_impl_call_stmt_test.cc
      reader/wgsl/parser_impl_compound_stmt_test.cc
      reader/wgsl/parser_impl_const_literal_test.cc
      reader/wgsl/parser_impl_continue_stmt_test.cc
      reader/wgsl/parser_impl_continuing_stmt_test.cc
      reader/wgsl/parser_impl_core_lhs_expression_test.cc
      reader/wgsl/parser_impl_diagnostic_attribute_test.cc
      reader/wgsl/parser_impl_diagnostic_control_test.cc
      reader/wgsl/parser_impl_diagnostic_directive_test.cc
      reader/wgsl/parser_impl_enable_directive_test.cc
      reader/wgsl/parser_impl_error_msg_test.cc
      reader/wgsl/parser_impl_error_resync_test.cc
      reader/wgsl/parser_impl_expression_test.cc
      reader/wgsl/parser_impl_for_stmt_test.cc
      reader/wgsl/parser_impl_function_decl_test.cc
      reader/wgsl/parser_impl_function_attribute_list_test.cc
      reader/wgsl/parser_impl_function_attribute_test.cc
      reader/wgsl/parser_impl_function_header_test.cc
      reader/wgsl/parser_impl_global_constant_decl_test.cc
      reader/wgsl/parser_impl_global_decl_test.cc
      reader/wgsl/parser_impl_global_variable_decl_test.cc
      reader/wgsl/parser_impl_if_stmt_test.cc
      reader/wgsl/parser_impl_increment_decrement_stmt_test.cc
      reader/wgsl/parser_impl_lhs_expression_test.cc
      reader/wgsl/parser_impl_loop_stmt_test.cc
      reader/wgsl/parser_impl_math_expression_test.cc
      reader/wgsl/parser_impl_multiplicative_expression_test.cc
      reader/wgsl/parser_impl_param_list_test.cc
      reader/wgsl/parser_impl_paren_expression_test.cc
      reader/wgsl/parser_impl_primary_expression_test.cc
      reader/wgsl/parser_impl_relational_expression_test.cc
      reader/wgsl/parser_impl_reserved_keyword_test.cc
      reader/wgsl/parser_impl_require_directive_test.cc
      reader/wgsl/parser_impl_shift_expression_test.cc
      reader/wgsl/parser_impl_singular_expression_test.cc
      reader/wgsl/parser_impl_statement_test.cc
      reader/wgsl/parser_impl_statements_test.cc
      reader/wgsl/parser_impl_struct_body_decl_test.cc
      reader/wgsl/parser_impl_struct_decl_test.cc
      reader/wgsl/parser_impl_struct_attribute_decl_test.cc
      reader/wgsl/parser_impl_struct_member_attribute_decl_test.cc
      reader/wgsl/parser_impl_struct_member_attribute_test.cc
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
      reader/wgsl/parser_impl_variable_attribute_list_test.cc
      reader/wgsl/parser_impl_variable_attribute_test.cc
      reader/wgsl/parser_impl_variable_ident_decl_test.cc
      reader/wgsl/parser_impl_variable_stmt_test.cc
      reader/wgsl/parser_impl_variable_qualifier_test.cc
      reader/wgsl/parser_impl_while_stmt_test.cc
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
      writer/spirv/builder_builtin_test.cc
      writer/spirv/builder_builtin_texture_test.cc
      writer/spirv/builder_call_test.cc
      writer/spirv/builder_const_assert_test.cc
      writer/spirv/builder_constructor_expression_test.cc
      writer/spirv/builder_discard_test.cc
      writer/spirv/builder_entry_point_test.cc
      writer/spirv/builder_format_conversion_test.cc
      writer/spirv/builder_function_attribute_test.cc
      writer/spirv/builder_function_test.cc
      writer/spirv/builder_function_variable_test.cc
      writer/spirv/builder_global_variable_test.cc
      writer/spirv/builder_ident_expression_test.cc
      writer/spirv/builder_if_test.cc
      writer/spirv/builder_literal_test.cc
      writer/spirv/builder_loop_test.cc
      writer/spirv/builder_return_test.cc
      writer/spirv/builder_switch_test.cc
      writer/spirv/builder_test.cc
      writer/spirv/builder_type_test.cc
      writer/spirv/builder_unary_op_expression_test.cc
      writer/spirv/instruction_test.cc
      writer/spirv/module_test.cc
      writer/spirv/operand_test.cc
      writer/spirv/scalar_constant_test.cc
      writer/spirv/spv_dump.cc
      writer/spirv/spv_dump.h
      writer/spirv/test_helper.h
    )

    if(${TINT_BUILD_IR})
      list(APPEND TINT_TEST_SRCS
        writer/spirv/generator_impl_binary_test.cc
        writer/spirv/generator_impl_constant_test.cc
        writer/spirv/generator_impl_function_test.cc
        writer/spirv/generator_impl_ir_test.cc
        writer/spirv/generator_impl_type_test.cc
        writer/spirv/test_helper_ir.h
      )
    endif()
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
      writer/wgsl/generator_impl_const_assert_test.cc
      writer/wgsl/generator_impl_constructor_test.cc
      writer/wgsl/generator_impl_continue_test.cc
      writer/wgsl/generator_impl_diagnostic_test.cc
      writer/wgsl/generator_impl_discard_test.cc
      writer/wgsl/generator_impl_enable_test.cc
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
      ast/module_clone_test.cc
      transform/add_empty_entry_point_test.cc
      transform/add_block_attribute_test.cc
      transform/array_length_from_uniform_test.cc
      transform/binding_remapper_test.cc
      transform/builtin_polyfill_test.cc
      transform/calculate_array_length_test.cc
      transform/clamp_frag_depth_test.cc
      transform/canonicalize_entry_point_io_test.cc
      transform/combine_samplers_test.cc
      transform/decompose_memory_access_test.cc
      transform/decompose_strided_array_test.cc
      transform/decompose_strided_matrix_test.cc
      transform/demote_to_helper_test.cc
      transform/direct_variable_access_test.cc
      transform/disable_uniformity_analysis_test.cc
      transform/expand_compound_assignment_test.cc
      transform/first_index_offset_test.cc
      transform/for_loop_to_loop_test.cc
      transform/expand_compound_assignment_test.cc
      transform/localize_struct_array_assignment_test.cc
      transform/merge_return_test.cc
      transform/module_scope_var_to_entry_point_param_test.cc
      transform/multiplanar_external_texture_test.cc
      transform/num_workgroups_from_uniform_test.cc
      transform/packed_vec3_test.cc
      transform/pad_structs_test.cc
      transform/preserve_padding_test.cc
      transform/promote_initializers_to_let_test.cc
      transform/promote_side_effects_to_decl_test.cc
      transform/remove_continue_in_switch_test.cc
      transform/remove_phonies_test.cc
      transform/remove_unreachable_statements_test.cc
      transform/renamer_test.cc
      transform/robustness_test.cc
      transform/simplify_pointers_test.cc
      transform/single_entry_point_test.cc
      transform/spirv_atomic_test.cc
      transform/std140_exhaustive_test.cc
      transform/std140_f16_test.cc
      transform/std140_f32_test.cc
      transform/std140_test.cc
      transform/substitute_override_test.cc
      transform/test_helper.h
      transform/texture_1d_to_2d_test.cc
      transform/truncate_interstage_variables_test.cc
      transform/unshadow_test.cc
      transform/var_for_dynamic_index_test.cc
      transform/vectorize_matrix_conversions_test.cc
      transform/vectorize_scalar_matrix_initializers_test.cc
      transform/vertex_pulling_test.cc
      transform/while_to_loop_test.cc
      transform/zero_init_workgroup_memory_test.cc
      transform/utils/get_insertion_point_test.cc
      transform/utils/hoist_to_decl_before_test.cc
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
      writer/msl/generator_impl_builtin_test.cc
      writer/msl/generator_impl_builtin_texture_test.cc
      writer/msl/generator_impl_call_test.cc
      writer/msl/generator_impl_case_test.cc
      writer/msl/generator_impl_cast_test.cc
      writer/msl/generator_impl_const_assert_test.cc
      writer/msl/generator_impl_constructor_test.cc
      writer/msl/generator_impl_continue_test.cc
      writer/msl/generator_impl_discard_test.cc
      writer/msl/generator_impl_function_test.cc
      writer/msl/generator_impl_identifier_test.cc
      writer/msl/generator_impl_if_test.cc
      writer/msl/generator_impl_import_test.cc
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
      writer/glsl/generator_impl_builtin_test.cc
      writer/glsl/generator_impl_builtin_texture_test.cc
      writer/glsl/generator_impl_call_test.cc
      writer/glsl/generator_impl_case_test.cc
      writer/glsl/generator_impl_cast_test.cc
      writer/glsl/generator_impl_constructor_test.cc
      writer/glsl/generator_impl_continue_test.cc
      writer/glsl/generator_impl_discard_test.cc
      writer/glsl/generator_impl_function_test.cc
      writer/glsl/generator_impl_identifier_test.cc
      writer/glsl/generator_impl_if_test.cc
      writer/glsl/generator_impl_import_test.cc
      writer/glsl/generator_impl_loop_test.cc
      writer/glsl/generator_impl_member_accessor_test.cc
      writer/glsl/generator_impl_module_constant_test.cc
      writer/glsl/generator_impl_return_test.cc
      writer/glsl/generator_impl_sanitizer_test.cc
      writer/glsl/generator_impl_storage_buffer_test.cc
      writer/glsl/generator_impl_switch_test.cc
      writer/glsl/generator_impl_test.cc
      writer/glsl/generator_impl_type_test.cc
      writer/glsl/generator_impl_unary_op_test.cc
      writer/glsl/generator_impl_uniform_buffer_test.cc
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
      writer/hlsl/generator_impl_builtin_test.cc
      writer/hlsl/generator_impl_builtin_texture_test.cc
      writer/hlsl/generator_impl_call_test.cc
      writer/hlsl/generator_impl_case_test.cc
      writer/hlsl/generator_impl_cast_test.cc
      writer/hlsl/generator_impl_const_assert_test.cc
      writer/hlsl/generator_impl_constructor_test.cc
      writer/hlsl/generator_impl_continue_test.cc
      writer/hlsl/generator_impl_discard_test.cc
      writer/hlsl/generator_impl_function_test.cc
      writer/hlsl/generator_impl_identifier_test.cc
      writer/hlsl/generator_impl_if_test.cc
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

  if (${TINT_BUILD_IR})
    list(APPEND TINT_TEST_SRCS
      ir/binary_test.cc
      ir/bitcast_test.cc
      ir/builder_impl_binary_test.cc
      ir/builder_impl_call_test.cc
      ir/builder_impl_literal_test.cc
      ir/builder_impl_materialize_test.cc
      ir/builder_impl_store_test.cc
      ir/builder_impl_test.cc
      ir/builder_impl_unary_test.cc
      ir/builder_impl_var_test.cc
      ir/constant_test.cc
      ir/discard_test.cc
      ir/module_test.cc
      ir/store_test.cc
      ir/test_helper.h
      ir/unary_test.cc
    )
  endif()

  if (${TINT_BUILD_FUZZERS})
    list(APPEND TINT_TEST_SRCS
      fuzzers/mersenne_twister_engine.cc
      fuzzers/mersenne_twister_engine.h
      fuzzers/random_generator.cc
      fuzzers/random_generator.h
      fuzzers/random_generator_engine.cc
      fuzzers/random_generator_engine.h
      fuzzers/random_generator_test.cc
    )
  endif()

  add_executable(tint_unittests ${TINT_TEST_SRCS})
  set_target_properties(${target} PROPERTIES FOLDER "Tests")

  if(MSVC)
    # TODO(crbug.com/tint/1749): MSVC debug builds can suffer from stack
    # overflows when resolving deeply nested expression chains or statements.
    # Production builds neither use MSVC nor debug, so just bump the stack size
    # for this build combination.
    if (IS_DEBUG_BUILD)
      target_link_options(tint_unittests PRIVATE "/STACK:4194304") # 4MB, default is 1MB
    endif()
  else()
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
endif(TINT_BUILD_TESTS)

################################################################################
# Benchmarks
################################################################################
if(TINT_BUILD_BENCHMARKS)
  if(NOT TINT_BUILD_WGSL_READER)
    message(FATAL_ERROR "TINT_BUILD_BENCHMARKS requires TINT_BUILD_WGSL_READER")
  endif()

  list(APPEND TINT_BENCHMARK_SRCS
    "switch_bench.cc"
    "bench/benchmark.cc"
    "reader/wgsl/parser_bench.cc"
  )

  if (${TINT_BUILD_GLSL_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS writer/glsl/generator_bench.cc)
  endif()
  if (${TINT_BUILD_HLSL_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS writer/hlsl/generator_bench.cc)
  endif()
  if (${TINT_BUILD_MSL_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS writer/msl/generator_bench.cc)
  endif()
  if (${TINT_BUILD_SPV_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS writer/spirv/generator_bench.cc)
  endif()
  if (${TINT_BUILD_WGSL_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS writer/wgsl/generator_bench.cc)
  endif()

  add_executable(tint-benchmark ${TINT_BENCHMARK_SRCS})
  set_target_properties(${target} PROPERTIES FOLDER "Benchmarks")

  tint_core_compile_options(tint-benchmark)

  target_link_libraries(tint-benchmark PRIVATE benchmark::benchmark libtint)

  if (TINT_EXTERNAL_BENCHMARK_CORPUS_DIR)
    # Glob all the files at TINT_EXTERNAL_BENCHMARK_CORPUS_DIR, and create a header
    # that lists these with the TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS() macro
    set(TINT_BENCHMARK_GEN_DIR "${DAWN_BUILD_GEN_DIR}/src/tint/benchmark/")
    set(TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAM_HEADER "${TINT_BENCHMARK_GEN_DIR}/external_wgsl_programs.h")
    message("Globbing ${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}...")
    file(GLOB_RECURSE
      TINT_EXTERNAL_BENCHMARK_FILES
      RELATIVE "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}"
      "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/**.wgsl")
    list(TRANSFORM TINT_EXTERNAL_BENCHMARK_FILES REPLACE
      "(.+)"
      "    BENCHMARK_CAPTURE\(FUNC, \"\\1\", \"${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/\\1\")")
    list(JOIN TINT_EXTERNAL_BENCHMARK_FILES "; \\\n" TINT_EXTERNAL_BENCHMARK_FILES)
    file(CONFIGURE
      OUTPUT "${TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAM_HEADER}"
      CONTENT "#define TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS(FUNC) \\
${TINT_EXTERNAL_BENCHMARK_FILES};")
    # Define TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAM_HEADER to the generated header path
    target_compile_definitions(tint-benchmark PRIVATE
      "TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAM_HEADER=\"${TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAM_HEADER}\"")
  endif()
endif(TINT_BUILD_BENCHMARKS)
