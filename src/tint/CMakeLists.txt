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
  utils/diagnostic/diagnostic.cc
  utils/diagnostic/diagnostic.h
  utils/diagnostic/formatter.cc
  utils/diagnostic/formatter.h
  utils/diagnostic/printer.cc
  utils/diagnostic/printer.h
  utils/diagnostic/source.cc
  utils/diagnostic/source.h
  utils/debug/debugger.cc
  utils/debug/debugger.h
  utils/ice/ice.cc
  utils/ice/ice.h
  utils/text/unicode.cc
  utils/text/unicode.h
)
tint_default_compile_options(tint_diagnostic_utils)

if (TINT_ENABLE_BREAK_IN_DEBUGGER)
  set_source_files_properties(utils/debug/debugger.cc
    PROPERTIES COMPILE_DEFINITIONS "TINT_ENABLE_BREAK_IN_DEBUGGER=1" )
endif()

list(APPEND TINT_LIB_SRCS
  ../../include/tint/tint.h
  lang/wgsl/ast/clone_context.cc
  lang/wgsl/ast/clone_context.h
  lang/core/builtin/fluent_types.h
  lang/core/builtin/number.cc
  lang/core/builtin/number.h
  lang/core/constant/clone_context.h
  lang/core/constant/composite.cc
  lang/core/constant/composite.h
  lang/core/constant/manager.cc
  lang/core/constant/manager.h
  lang/core/constant/node.cc
  lang/core/constant/node.h
  lang/core/constant/scalar.cc
  lang/core/constant/scalar.h
  lang/core/constant/splat.cc
  lang/core/constant/splat.h
  lang/core/constant/value.cc
  lang/core/constant/value.h
  lang/core/type/abstract_float.cc
  lang/core/type/abstract_float.h
  lang/core/type/abstract_int.cc
  lang/core/type/abstract_int.h
  lang/core/type/abstract_numeric.cc
  lang/core/type/abstract_numeric.h
  lang/core/type/array.cc
  lang/core/type/array.h
  lang/core/type/array_count.cc
  lang/core/type/array_count.h
  lang/core/type/atomic.cc
  lang/core/type/atomic.h
  lang/core/type/bool.cc
  lang/core/type/bool.h
  lang/core/type/builtin_structs.cc
  lang/core/type/builtin_structs.h
  lang/core/type/clone_context.h
  lang/core/type/depth_multisampled_texture.cc
  lang/core/type/depth_multisampled_texture.h
  lang/core/type/depth_texture.cc
  lang/core/type/depth_texture.h
  lang/core/type/external_texture.cc
  lang/core/type/external_texture.h
  lang/core/type/f16.cc
  lang/core/type/f16.h
  lang/core/type/f32.cc
  lang/core/type/f32.h
  lang/core/type/i32.cc
  lang/core/type/i32.h
  lang/core/type/manager.cc
  lang/core/type/manager.h
  lang/core/type/matrix.cc
  lang/core/type/matrix.h
  lang/core/type/multisampled_texture.cc
  lang/core/type/multisampled_texture.h
  lang/core/type/node.cc
  lang/core/type/node.h
  lang/core/type/numeric_scalar.cc
  lang/core/type/numeric_scalar.h
  lang/core/type/pointer.cc
  lang/core/type/pointer.h
  lang/core/type/reference.cc
  lang/core/type/reference.h
  lang/core/type/sampled_texture.cc
  lang/core/type/sampled_texture.h
  lang/core/type/sampler.cc
  lang/core/type/sampler.h
  lang/core/type/sampler_kind.cc
  lang/core/type/sampler_kind.h
  lang/core/type/scalar.cc
  lang/core/type/scalar.h
  lang/core/type/storage_texture.cc
  lang/core/type/storage_texture.h
  lang/core/type/struct.cc
  lang/core/type/struct.h
  lang/core/type/texture.cc
  lang/core/type/texture.h
  lang/core/type/texture_dimension.cc
  lang/core/type/texture_dimension.h
  lang/core/type/type.cc
  lang/core/type/type.h
  lang/core/type/u32.cc
  lang/core/type/u32.h
  lang/core/type/unique_node.cc
  lang/core/type/unique_node.h
  lang/core/type/vector.cc
  lang/core/type/vector.h
  lang/core/type/void.cc
  lang/core/type/void.h
  lang/wgsl/ast/accessor_expression.cc
  lang/wgsl/ast/accessor_expression.h
  lang/wgsl/ast/alias.cc
  lang/wgsl/ast/alias.h
  lang/wgsl/ast/assignment_statement.cc
  lang/wgsl/ast/assignment_statement.h
  lang/wgsl/ast/attribute.cc
  lang/wgsl/ast/attribute.h
  lang/wgsl/ast/binary_expression.cc
  lang/wgsl/ast/binary_expression.h
  lang/wgsl/ast/binding_attribute.cc
  lang/wgsl/ast/binding_attribute.h
  lang/wgsl/ast/bitcast_expression.cc
  lang/wgsl/ast/bitcast_expression.h
  lang/wgsl/ast/block_statement.cc
  lang/wgsl/ast/block_statement.h
  lang/wgsl/ast/bool_literal_expression.cc
  lang/wgsl/ast/bool_literal_expression.h
  lang/wgsl/ast/break_if_statement.cc
  lang/wgsl/ast/break_if_statement.h
  lang/wgsl/ast/break_statement.cc
  lang/wgsl/ast/break_statement.h
  lang/wgsl/ast/builder.cc
  lang/wgsl/ast/builder.h
  lang/wgsl/ast/builtin_attribute.cc
  lang/wgsl/ast/builtin_attribute.h
  lang/wgsl/ast/call_expression.cc
  lang/wgsl/ast/call_expression.h
  lang/wgsl/ast/call_statement.cc
  lang/wgsl/ast/call_statement.h
  lang/wgsl/ast/case_selector.cc
  lang/wgsl/ast/case_selector.h
  lang/wgsl/ast/case_statement.cc
  lang/wgsl/ast/case_statement.h
  lang/wgsl/ast/compound_assignment_statement.cc
  lang/wgsl/ast/compound_assignment_statement.h
  lang/wgsl/ast/const.cc
  lang/wgsl/ast/const.h
  lang/wgsl/ast/const_assert.cc
  lang/wgsl/ast/const_assert.h
  lang/wgsl/ast/continue_statement.cc
  lang/wgsl/ast/continue_statement.h
  lang/wgsl/ast/diagnostic_attribute.cc
  lang/wgsl/ast/diagnostic_attribute.h
  lang/wgsl/ast/diagnostic_control.cc
  lang/wgsl/ast/diagnostic_control.h
  lang/wgsl/ast/diagnostic_directive.cc
  lang/wgsl/ast/diagnostic_directive.h
  lang/wgsl/ast/diagnostic_rule_name.cc
  lang/wgsl/ast/diagnostic_rule_name.h
  lang/wgsl/ast/disable_validation_attribute.cc
  lang/wgsl/ast/disable_validation_attribute.h
  lang/wgsl/ast/discard_statement.cc
  lang/wgsl/ast/discard_statement.h
  lang/wgsl/ast/enable.cc
  lang/wgsl/ast/enable.h
  lang/wgsl/ast/expression.cc
  lang/wgsl/ast/expression.h
  lang/wgsl/ast/extension.cc
  lang/wgsl/ast/extension.h
  lang/wgsl/ast/float_literal_expression.cc
  lang/wgsl/ast/float_literal_expression.h
  lang/wgsl/ast/for_loop_statement.cc
  lang/wgsl/ast/for_loop_statement.h
  lang/wgsl/ast/function.cc
  lang/wgsl/ast/function.h
  lang/wgsl/ast/group_attribute.cc
  lang/wgsl/ast/group_attribute.h
  lang/wgsl/ast/id_attribute.cc
  lang/wgsl/ast/id_attribute.h
  lang/wgsl/ast/identifier.cc
  lang/wgsl/ast/identifier.h
  lang/wgsl/ast/identifier_expression.cc
  lang/wgsl/ast/identifier_expression.h
  lang/wgsl/ast/if_statement.cc
  lang/wgsl/ast/if_statement.h
  lang/wgsl/ast/increment_decrement_statement.cc
  lang/wgsl/ast/increment_decrement_statement.h
  lang/wgsl/ast/index_accessor_expression.cc
  lang/wgsl/ast/index_accessor_expression.h
  lang/wgsl/ast/index_attribute.cc
  lang/wgsl/ast/index_attribute.h
  lang/wgsl/ast/int_literal_expression.cc
  lang/wgsl/ast/int_literal_expression.h
  lang/wgsl/ast/internal_attribute.cc
  lang/wgsl/ast/internal_attribute.h
  lang/wgsl/ast/interpolate_attribute.cc
  lang/wgsl/ast/interpolate_attribute.h
  lang/wgsl/ast/invariant_attribute.cc
  lang/wgsl/ast/invariant_attribute.h
  lang/wgsl/ast/let.cc
  lang/wgsl/ast/let.h
  lang/wgsl/ast/literal_expression.cc
  lang/wgsl/ast/literal_expression.h
  lang/wgsl/ast/location_attribute.cc
  lang/wgsl/ast/location_attribute.h
  lang/wgsl/ast/loop_statement.cc
  lang/wgsl/ast/loop_statement.h
  lang/wgsl/ast/member_accessor_expression.cc
  lang/wgsl/ast/member_accessor_expression.h
  lang/wgsl/ast/module.cc
  lang/wgsl/ast/module.h
  lang/wgsl/ast/must_use_attribute.cc
  lang/wgsl/ast/must_use_attribute.h
  lang/wgsl/ast/node.cc
  lang/wgsl/ast/node.h
  lang/wgsl/ast/node_id.h
  lang/wgsl/ast/override.cc
  lang/wgsl/ast/override.h
  lang/wgsl/ast/parameter.cc
  lang/wgsl/ast/parameter.h
  lang/wgsl/ast/phony_expression.cc
  lang/wgsl/ast/phony_expression.h
  lang/wgsl/ast/pipeline_stage.cc
  lang/wgsl/ast/pipeline_stage.h
  lang/wgsl/ast/return_statement.cc
  lang/wgsl/ast/return_statement.h
  lang/wgsl/ast/stage_attribute.cc
  lang/wgsl/ast/stage_attribute.h
  lang/wgsl/ast/statement.cc
  lang/wgsl/ast/statement.h
  lang/wgsl/ast/stride_attribute.cc
  lang/wgsl/ast/stride_attribute.h
  lang/wgsl/ast/struct.cc
  lang/wgsl/ast/struct.h
  lang/wgsl/ast/struct_member.cc
  lang/wgsl/ast/struct_member.h
  lang/wgsl/ast/struct_member_align_attribute.cc
  lang/wgsl/ast/struct_member_align_attribute.h
  lang/wgsl/ast/struct_member_offset_attribute.cc
  lang/wgsl/ast/struct_member_offset_attribute.h
  lang/wgsl/ast/struct_member_size_attribute.cc
  lang/wgsl/ast/struct_member_size_attribute.h
  lang/wgsl/ast/switch_statement.cc
  lang/wgsl/ast/switch_statement.h
  lang/wgsl/ast/templated_identifier.cc
  lang/wgsl/ast/templated_identifier.h
  lang/wgsl/ast/transform/add_block_attribute.cc
  lang/wgsl/ast/transform/add_block_attribute.h
  lang/wgsl/ast/transform/add_empty_entry_point.cc
  lang/wgsl/ast/transform/add_empty_entry_point.h
  lang/wgsl/ast/transform/array_length_from_uniform.cc
  lang/wgsl/ast/transform/array_length_from_uniform.h
  lang/wgsl/ast/transform/binding_remapper.cc
  lang/wgsl/ast/transform/binding_remapper.h
  lang/wgsl/ast/transform/builtin_polyfill.cc
  lang/wgsl/ast/transform/builtin_polyfill.h
  lang/wgsl/ast/transform/calculate_array_length.cc
  lang/wgsl/ast/transform/calculate_array_length.h
  lang/wgsl/ast/transform/canonicalize_entry_point_io.cc
  lang/wgsl/ast/transform/canonicalize_entry_point_io.h
  lang/wgsl/ast/transform/clamp_frag_depth.cc
  lang/wgsl/ast/transform/clamp_frag_depth.h
  lang/wgsl/ast/transform/combine_samplers.cc
  lang/wgsl/ast/transform/combine_samplers.h
  lang/wgsl/ast/transform/data.cc
  lang/wgsl/ast/transform/data.h
  lang/wgsl/ast/transform/decompose_memory_access.cc
  lang/wgsl/ast/transform/decompose_memory_access.h
  lang/wgsl/ast/transform/decompose_strided_array.cc
  lang/wgsl/ast/transform/decompose_strided_array.h
  lang/wgsl/ast/transform/decompose_strided_matrix.cc
  lang/wgsl/ast/transform/decompose_strided_matrix.h
  lang/wgsl/ast/transform/demote_to_helper.cc
  lang/wgsl/ast/transform/demote_to_helper.h
  lang/wgsl/ast/transform/direct_variable_access.cc
  lang/wgsl/ast/transform/direct_variable_access.h
  lang/wgsl/ast/transform/disable_uniformity_analysis.cc
  lang/wgsl/ast/transform/disable_uniformity_analysis.h
  lang/wgsl/ast/transform/expand_compound_assignment.cc
  lang/wgsl/ast/transform/expand_compound_assignment.h
  lang/wgsl/ast/transform/first_index_offset.cc
  lang/wgsl/ast/transform/first_index_offset.h
  lang/wgsl/ast/transform/fold_trivial_lets.cc
  lang/wgsl/ast/transform/fold_trivial_lets.h
  lang/wgsl/ast/transform/for_loop_to_loop.cc
  lang/wgsl/ast/transform/for_loop_to_loop.h
  lang/wgsl/ast/transform/localize_struct_array_assignment.cc
  lang/wgsl/ast/transform/localize_struct_array_assignment.h
  lang/wgsl/ast/transform/manager.cc
  lang/wgsl/ast/transform/manager.h
  lang/wgsl/ast/transform/merge_return.cc
  lang/wgsl/ast/transform/merge_return.h
  lang/wgsl/ast/transform/module_scope_var_to_entry_point_param.cc
  lang/wgsl/ast/transform/module_scope_var_to_entry_point_param.h
  lang/wgsl/ast/transform/multiplanar_external_texture.cc
  lang/wgsl/ast/transform/multiplanar_external_texture.h
  lang/wgsl/ast/transform/num_workgroups_from_uniform.cc
  lang/wgsl/ast/transform/num_workgroups_from_uniform.h
  lang/wgsl/ast/transform/packed_vec3.cc
  lang/wgsl/ast/transform/packed_vec3.h
  lang/wgsl/ast/transform/pad_structs.cc
  lang/wgsl/ast/transform/pad_structs.h
  lang/wgsl/ast/transform/preserve_padding.cc
  lang/wgsl/ast/transform/preserve_padding.h
  lang/wgsl/ast/transform/promote_initializers_to_let.cc
  lang/wgsl/ast/transform/promote_initializers_to_let.h
  lang/wgsl/ast/transform/promote_side_effects_to_decl.cc
  lang/wgsl/ast/transform/promote_side_effects_to_decl.h
  lang/wgsl/ast/transform/remove_continue_in_switch.cc
  lang/wgsl/ast/transform/remove_continue_in_switch.h
  lang/wgsl/ast/transform/remove_phonies.cc
  lang/wgsl/ast/transform/remove_phonies.h
  lang/wgsl/ast/transform/remove_unreachable_statements.cc
  lang/wgsl/ast/transform/remove_unreachable_statements.h
  lang/wgsl/ast/transform/renamer.cc
  lang/wgsl/ast/transform/renamer.h
  lang/wgsl/ast/transform/robustness.cc
  lang/wgsl/ast/transform/robustness.h
  lang/wgsl/ast/transform/simplify_pointers.cc
  lang/wgsl/ast/transform/simplify_pointers.h
  lang/wgsl/ast/transform/single_entry_point.cc
  lang/wgsl/ast/transform/single_entry_point.h
  lang/wgsl/ast/transform/spirv_atomic.cc
  lang/wgsl/ast/transform/spirv_atomic.h
  lang/wgsl/ast/transform/std140.cc
  lang/wgsl/ast/transform/std140.h
  lang/wgsl/ast/transform/substitute_override.cc
  lang/wgsl/ast/transform/substitute_override.h
  lang/wgsl/ast/transform/texture_1d_to_2d.cc
  lang/wgsl/ast/transform/texture_1d_to_2d.h
  lang/wgsl/ast/transform/transform.cc
  lang/wgsl/ast/transform/transform.h
  lang/wgsl/ast/transform/truncate_interstage_variables.cc
  lang/wgsl/ast/transform/truncate_interstage_variables.h
  lang/wgsl/ast/transform/unshadow.cc
  lang/wgsl/ast/transform/unshadow.h
  lang/wgsl/ast/transform/utils/get_insertion_point.cc
  lang/wgsl/ast/transform/utils/get_insertion_point.h
  lang/wgsl/ast/transform/utils/hoist_to_decl_before.cc
  lang/wgsl/ast/transform/utils/hoist_to_decl_before.h
  lang/wgsl/ast/transform/var_for_dynamic_index.cc
  lang/wgsl/ast/transform/var_for_dynamic_index.h
  lang/wgsl/ast/transform/vectorize_matrix_conversions.cc
  lang/wgsl/ast/transform/vectorize_matrix_conversions.h
  lang/wgsl/ast/transform/vectorize_scalar_matrix_initializers.cc
  lang/wgsl/ast/transform/vectorize_scalar_matrix_initializers.h
  lang/wgsl/ast/transform/vertex_pulling.cc
  lang/wgsl/ast/transform/vertex_pulling.h
  lang/wgsl/ast/transform/while_to_loop.cc
  lang/wgsl/ast/transform/while_to_loop.h
  lang/wgsl/ast/transform/zero_init_workgroup_memory.cc
  lang/wgsl/ast/transform/zero_init_workgroup_memory.h
  lang/wgsl/ast/traverse_expressions.h
  lang/wgsl/ast/type.cc
  lang/wgsl/ast/type.h
  lang/wgsl/ast/type_decl.cc
  lang/wgsl/ast/type_decl.h
  lang/wgsl/ast/unary_op.cc
  lang/wgsl/ast/unary_op.h
  lang/wgsl/ast/unary_op_expression.cc
  lang/wgsl/ast/unary_op_expression.h
  lang/wgsl/ast/var.cc
  lang/wgsl/ast/var.h
  lang/wgsl/ast/variable.cc
  lang/wgsl/ast/variable.h
  lang/wgsl/ast/variable_decl_statement.cc
  lang/wgsl/ast/variable_decl_statement.h
  lang/wgsl/ast/while_statement.cc
  lang/wgsl/ast/while_statement.h
  lang/wgsl/ast/workgroup_attribute.cc
  lang/wgsl/ast/workgroup_attribute.h
  lang/wgsl/helpers/append_vector.cc
  lang/wgsl/helpers/append_vector.h
  lang/wgsl/helpers/check_supported_extensions.cc
  lang/wgsl/helpers/check_supported_extensions.h
  lang/wgsl/helpers/flatten_bindings.cc
  lang/wgsl/helpers/flatten_bindings.h
  lang/wgsl/inspector/entry_point.cc
  lang/wgsl/inspector/entry_point.h
  lang/wgsl/inspector/inspector.cc
  lang/wgsl/inspector/inspector.h
  lang/wgsl/inspector/resource_binding.cc
  lang/wgsl/inspector/resource_binding.h
  lang/wgsl/inspector/scalar.cc
  lang/wgsl/inspector/scalar.h
  lang/wgsl/program/clone_context.cc
  lang/wgsl/program/clone_context.h
  lang/wgsl/program/program.cc
  lang/wgsl/program/program.h
  lang/wgsl/program/program_builder.cc
  lang/wgsl/program/program_builder.h
  lang/wgsl/sem/accessor_expression.cc
  lang/wgsl/sem/accessor_expression.h
  lang/wgsl/sem/array_count.cc
  lang/wgsl/sem/array_count.h
  lang/wgsl/sem/behavior.cc
  lang/wgsl/sem/behavior.h
  lang/wgsl/sem/block_statement.cc
  lang/wgsl/sem/block_statement.h
  lang/wgsl/sem/break_if_statement.cc
  lang/wgsl/sem/break_if_statement.h
  lang/wgsl/sem/builtin.cc
  lang/wgsl/sem/builtin.h
  lang/wgsl/sem/builtin_enum_expression.cc
  lang/wgsl/sem/builtin_enum_expression.h
  lang/wgsl/sem/call.cc
  lang/wgsl/sem/call.h
  lang/wgsl/sem/call_target.cc
  lang/wgsl/sem/call_target.h
  lang/wgsl/sem/evaluation_stage.h
  lang/wgsl/sem/expression.cc
  lang/wgsl/sem/expression.h
  lang/wgsl/sem/external_texture.h
  lang/wgsl/sem/for_loop_statement.cc
  lang/wgsl/sem/for_loop_statement.h
  lang/wgsl/sem/function.cc
  lang/wgsl/sem/function_expression.cc
  lang/wgsl/sem/function_expression.h
  lang/wgsl/sem/if_statement.cc
  lang/wgsl/sem/if_statement.h
  lang/wgsl/sem/index_accessor_expression.cc
  lang/wgsl/sem/index_accessor_expression.h
  lang/wgsl/sem/info.cc
  lang/wgsl/sem/info.h
  lang/wgsl/sem/load.cc
  lang/wgsl/sem/load.h
  lang/wgsl/sem/loop_statement.cc
  lang/wgsl/sem/loop_statement.h
  lang/wgsl/sem/materialize.cc
  lang/wgsl/sem/materialize.h
  lang/wgsl/sem/member_accessor_expression.cc
  lang/wgsl/sem/module.cc
  lang/wgsl/sem/module.h
  lang/wgsl/sem/node.cc
  lang/wgsl/sem/node.h
  lang/wgsl/sem/pipeline_stage_set.h
  lang/wgsl/sem/sampler_texture_pair.h
  lang/wgsl/sem/statement.cc
  lang/wgsl/sem/struct.cc
  lang/wgsl/sem/struct.h
  lang/wgsl/sem/switch_statement.cc
  lang/wgsl/sem/switch_statement.h
  lang/wgsl/sem/type_expression.cc
  lang/wgsl/sem/type_expression.h
  lang/wgsl/sem/type_mappings.h
  lang/wgsl/sem/value_constructor.cc
  lang/wgsl/sem/value_constructor.h
  lang/wgsl/sem/value_conversion.cc
  lang/wgsl/sem/value_conversion.h
  lang/wgsl/sem/value_expression.cc
  lang/wgsl/sem/value_expression.h
  lang/wgsl/sem/variable.cc
  lang/wgsl/sem/while_statement.cc
  lang/wgsl/sem/while_statement.h
  lang/wgsl/resolver/const_eval.cc
  lang/wgsl/resolver/const_eval.h
  lang/wgsl/resolver/dependency_graph.cc
  lang/wgsl/resolver/dependency_graph.h
  lang/wgsl/resolver/intrinsic_table.cc
  lang/wgsl/resolver/intrinsic_table.h
  lang/wgsl/resolver/intrinsic_table.inl
  lang/wgsl/resolver/resolve.cc
  lang/wgsl/resolver/resolve.h
  lang/wgsl/resolver/resolver.cc
  lang/wgsl/resolver/resolver.h
  lang/wgsl/resolver/sem_helper.cc
  lang/wgsl/resolver/sem_helper.h
  lang/wgsl/resolver/uniformity.cc
  lang/wgsl/resolver/uniformity.h
  lang/wgsl/resolver/validator.cc
  lang/wgsl/resolver/validator.h
  tint.cc
  utils/cli/cli.cc
  utils/cli/cli.h
  utils/containers/bitset.h
  utils/containers/enum_set.h
  utils/containers/hashmap.h
  utils/containers/hashmap_base.h
  utils/containers/hashset.h
  utils/containers/map.h
  utils/containers/predicates.h
  utils/containers/scope_stack.h
  utils/containers/slice.h
  utils/containers/unique_allocator.h
  utils/containers/unique_vector.h
  utils/containers/vector.h
  utils/generation_id.cc
  utils/generation_id.h
  utils/macros/compiler.h
  utils/macros/concat.h
  utils/macros/foreach.h
  utils/macros/scoped_assignment.h
  utils/math/crc32.h
  utils/math/hash.h
  utils/math/math.h
  utils/memory/bitcast.h
  utils/memory/block_allocator.h
  utils/memory/bump_allocator.h
  utils/reflection/reflection.h
  utils/rtti/castable.cc
  utils/rtti/castable.h
  utils/rtti/switch.h
  utils/text/float_to_string.cc
  utils/text/float_to_string.h
  utils/text/parse_num.cc
  utils/text/parse_num.h
  utils/text/string.cc
  utils/text/string.h
  utils/text/string_stream.cc
  utils/text/string_stream.h
  utils/symbol/symbol.cc
  utils/symbol/symbol.h
  utils/symbol/symbol_table.cc
  utils/symbol/symbol_table.h
  utils/text/text_generator.cc
  utils/text/text_generator.h
  utils/traits/traits.h
)

tint_generated(lang/core/builtin/access BENCH TEST)
tint_generated(lang/core/builtin/address_space BENCH TEST)
tint_generated(lang/core/builtin/attribute BENCH TEST)
tint_generated(lang/core/builtin/builtin BENCH TEST)
tint_generated(lang/core/builtin/builtin_value BENCH TEST)
tint_generated(lang/core/builtin/diagnostic_rule BENCH TEST)
tint_generated(lang/core/builtin/diagnostic_severity BENCH TEST)
tint_generated(lang/core/builtin/extension BENCH TEST)
tint_generated(lang/core/builtin/function)
tint_generated(lang/core/builtin/interpolation_sampling BENCH TEST)
tint_generated(lang/core/builtin/interpolation_type BENCH TEST)
tint_generated(lang/core/builtin/texel_format BENCH TEST)

tint_generated(lang/wgsl/resolver/ctor_conv_intrinsic)
tint_generated(lang/wgsl/sem/parameter_usage)

if(UNIX)
  list(APPEND TINT_LIB_SRCS utils/diagnostic/printer_posix.cc)
elseif(WIN32)
  list(APPEND TINT_LIB_SRCS utils/diagnostic/printer_windows.cc)
else()
  list(APPEND TINT_LIB_SRCS utils/diagnostic/printer_other.cc)
endif()

if(${TINT_BUILD_SPV_READER})
  list(APPEND TINT_LIB_SRCS
    lang/spirv/reader/ast_parser/ast_parser.cc
    lang/spirv/reader/ast_parser/ast_parser.h
    lang/spirv/reader/ast_parser/attributes.h
    lang/spirv/reader/ast_parser/construct.cc
    lang/spirv/reader/ast_parser/construct.h
    lang/spirv/reader/ast_parser/entry_point_info.cc
    lang/spirv/reader/ast_parser/entry_point_info.h
    lang/spirv/reader/ast_parser/enum_converter.cc
    lang/spirv/reader/ast_parser/enum_converter.h
    lang/spirv/reader/ast_parser/fail_stream.h
    lang/spirv/reader/ast_parser/function.cc
    lang/spirv/reader/ast_parser/function.h
    lang/spirv/reader/ast_parser/namer.cc
    lang/spirv/reader/ast_parser/namer.h
    lang/spirv/reader/ast_parser/type.cc
    lang/spirv/reader/ast_parser/type.h
    lang/spirv/reader/ast_parser/usage.cc
    lang/spirv/reader/ast_parser/usage.h
    lang/spirv/reader/reader.cc
    lang/spirv/reader/reader.h
  )
endif()

if(${TINT_BUILD_WGSL_READER})
  list(APPEND TINT_LIB_SRCS
    lang/wgsl/reader/parser/classify_template_args.cc
    lang/wgsl/reader/parser/classify_template_args.h
    lang/wgsl/reader/parser/detail.h
    lang/wgsl/reader/parser/lexer.cc
    lang/wgsl/reader/parser/lexer.h
    lang/wgsl/reader/parser/parser.cc
    lang/wgsl/reader/parser/parser.h
    lang/wgsl/reader/parser/token.cc
    lang/wgsl/reader/parser/token.h
    lang/wgsl/reader/reader.cc
    lang/wgsl/reader/reader.h
  )

  if(${TINT_BUILD_IR})
    list(APPEND TINT_LIB_SRCS
      lang/wgsl/reader/program_to_ir/program_to_ir.cc
      lang/wgsl/reader/program_to_ir/program_to_ir.h
    )
  endif()
endif()

if(${TINT_BUILD_SPV_WRITER})
  list(APPEND TINT_LIB_SRCS
    lang/spirv/writer/binary_writer.cc
    lang/spirv/writer/binary_writer.h
    lang/spirv/writer/function.cc
    lang/spirv/writer/function.h
    lang/spirv/writer/instruction.cc
    lang/spirv/writer/instruction.h
    lang/spirv/writer/module.cc
    lang/spirv/writer/module.h
    lang/spirv/writer/operand.cc
    lang/spirv/writer/operand.h
    lang/spirv/writer/writer.cc
    lang/spirv/writer/writer.h
    lang/spirv/writer/ast_printer/builder.cc
    lang/spirv/writer/ast_printer/builder.h
    lang/spirv/writer/ast_printer/ast_printer.cc
    lang/spirv/writer/ast_printer/ast_printer.h
    lang/spirv/writer/ast_printer/scalar_constant.h
  )

  if(${TINT_BUILD_IR})
    list(APPEND TINT_LIB_SRCS
      lang/core/ir/transform/builtin_polyfill_spirv.cc
      lang/core/ir/transform/builtin_polyfill_spirv.h
      lang/core/ir/transform/expand_implicit_splats.cc
      lang/core/ir/transform/expand_implicit_splats.h
      lang/core/ir/transform/handle_matrix_arithmetic.cc
      lang/core/ir/transform/handle_matrix_arithmetic.h
      lang/core/ir/transform/merge_return.cc
      lang/core/ir/transform/merge_return.h
      lang/core/ir/transform/shader_io_spirv.cc
      lang/core/ir/transform/shader_io_spirv.h
      lang/core/ir/transform/std140.cc
      lang/core/ir/transform/std140.h
      lang/core/ir/transform/var_for_dynamic_index.cc
      lang/core/ir/transform/var_for_dynamic_index.h
      lang/spirv/writer/printer/printer.cc
      lang/spirv/writer/printer/printer.h
    )
  endif()
endif()

if(${TINT_BUILD_WGSL_WRITER})
  list(APPEND TINT_LIB_SRCS
    lang/wgsl/writer/options.cc
    lang/wgsl/writer/options.h
    lang/wgsl/writer/result.cc
    lang/wgsl/writer/result.h
    lang/wgsl/writer/writer.cc
    lang/wgsl/writer/writer.h
    lang/wgsl/writer/ast_printer/ast_printer.cc
    lang/wgsl/writer/ast_printer/ast_printer.h
  )

  if(${TINT_BUILD_IR})
    list(APPEND TINT_LIB_SRCS
      lang/wgsl/writer/ir_to_program/ir_to_program.cc
      lang/wgsl/writer/ir_to_program/ir_to_program.h
      lang/wgsl/writer/ir_to_program/rename_conflicts.cc
      lang/wgsl/writer/ir_to_program/rename_conflicts.h
    )
  endif()
endif()

if(${TINT_BUILD_MSL_WRITER})
  list(APPEND TINT_LIB_SRCS
    lang/msl/writer/options.cc
    lang/msl/writer/options.h
    lang/msl/writer/printer_support.cc
    lang/msl/writer/printer_support.h
    lang/msl/writer/result.cc
    lang/msl/writer/result.h
    lang/msl/writer/writer.cc
    lang/msl/writer/writer.h
    lang/msl/writer/ast_printer/ast_printer.cc
    lang/msl/writer/ast_printer/ast_printer.h
  )

  if(${TINT_BUILD_IR})
    list(APPEND TINT_LIB_SRCS
      lang/msl/writer/printer/printer.cc
      lang/msl/writer/printer/printer.h
    )
  endif()
endif()

if(${TINT_BUILD_GLSL_WRITER})
  list(APPEND TINT_LIB_SRCS
    lang/glsl/writer/ast_printer/ast_printer.cc
    lang/glsl/writer/ast_printer/ast_printer.h
    lang/glsl/writer/writer.cc
    lang/glsl/writer/writer.h
    lang/glsl/writer/options.cc
    lang/glsl/writer/options.h
    lang/glsl/writer/result.cc
    lang/glsl/writer/result.h
    lang/glsl/writer/version.h
  )
endif()

if(${TINT_BUILD_HLSL_WRITER})
  list(APPEND TINT_LIB_SRCS
    lang/hlsl/writer/options.cc
    lang/hlsl/writer/options.h
    lang/hlsl/writer/result.cc
    lang/hlsl/writer/result.h
    lang/hlsl/writer/writer.cc
    lang/hlsl/writer/writer.h
    lang/hlsl/writer/ast_printer/ast_printer.cc
    lang/hlsl/writer/ast_printer/ast_printer.h
  )
endif()

if(${TINT_BUILD_SYNTAX_TREE_WRITER})
    list(APPEND TINT_LIB_SRCS
      lang/wgsl/writer/syntax_tree_printer/syntax_tree_printer.cc
      lang/wgsl/writer/syntax_tree_printer/syntax_tree_printer.h
    )
endif()

if(${TINT_BUILD_IR})
  list(APPEND TINT_LIB_SRCS
    lang/core/ir/access.cc
    lang/core/ir/access.h
    lang/core/ir/binary.cc
    lang/core/ir/binary.h
    lang/core/ir/binding_point.h
    lang/core/ir/bitcast.cc
    lang/core/ir/bitcast.h
    lang/core/ir/block.cc
    lang/core/ir/block.h
    lang/core/ir/block_param.cc
    lang/core/ir/block_param.h
    lang/core/ir/break_if.cc
    lang/core/ir/break_if.h
    lang/core/ir/builder.cc
    lang/core/ir/builder.h
    lang/core/ir/builtin_call.cc
    lang/core/ir/builtin_call.h
    lang/core/ir/call.cc
    lang/core/ir/call.h
    lang/core/ir/constant.cc
    lang/core/ir/constant.h
    lang/core/ir/construct.cc
    lang/core/ir/construct.h
    lang/core/ir/continue.cc
    lang/core/ir/continue.h
    lang/core/ir/control_instruction.cc
    lang/core/ir/control_instruction.h
    lang/core/ir/convert.cc
    lang/core/ir/convert.h
    lang/core/ir/core_builtin_call.cc
    lang/core/ir/core_builtin_call.h
    lang/core/ir/disassembler.cc
    lang/core/ir/disassembler.h
    lang/core/ir/discard.cc
    lang/core/ir/discard.h
    lang/core/ir/exit_if.cc
    lang/core/ir/exit_if.h
    lang/core/ir/exit_loop.cc
    lang/core/ir/exit_loop.h
    lang/core/ir/exit_switch.cc
    lang/core/ir/exit_switch.h
    lang/core/ir/exit.cc
    lang/core/ir/exit.h
    lang/core/ir/function.cc
    lang/core/ir/function.h
    lang/core/ir/function_param.cc
    lang/core/ir/function_param.h
    lang/core/ir/if.cc
    lang/core/ir/if.h
    lang/core/ir/instruction.cc
    lang/core/ir/instruction.h
    lang/core/ir/instruction_result.cc
    lang/core/ir/instruction_result.h
    lang/core/ir/intrinsic_call.cc
    lang/core/ir/intrinsic_call.h
    lang/core/ir/let.cc
    lang/core/ir/let.h
    lang/core/ir/load.cc
    lang/core/ir/load.h
    lang/core/ir/load_vector_element.cc
    lang/core/ir/load_vector_element.h
    lang/core/ir/location.h
    lang/core/ir/loop.cc
    lang/core/ir/loop.h
    lang/core/ir/module.cc
    lang/core/ir/module.h
    lang/core/ir/multi_in_block.cc
    lang/core/ir/multi_in_block.h
    lang/core/ir/next_iteration.cc
    lang/core/ir/next_iteration.h
    lang/core/ir/operand_instruction.cc
    lang/core/ir/operand_instruction.h
    lang/core/ir/return.cc
    lang/core/ir/return.h
    lang/core/ir/store.cc
    lang/core/ir/store.h
    lang/core/ir/store_vector_element.cc
    lang/core/ir/store_vector_element.h
    lang/core/ir/switch.cc
    lang/core/ir/switch.h
    lang/core/ir/swizzle.cc
    lang/core/ir/swizzle.h
    lang/core/ir/terminate_invocation.cc
    lang/core/ir/terminate_invocation.h
    lang/core/ir/terminator.cc
    lang/core/ir/terminator.h
    lang/core/ir/unary.cc
    lang/core/ir/unary.h
    lang/core/ir/unreachable.cc
    lang/core/ir/unreachable.h
    lang/core/ir/user_call.cc
    lang/core/ir/user_call.h
    lang/core/ir/validator.cc
    lang/core/ir/validator.h
    lang/core/ir/value.cc
    lang/core/ir/value.h
    lang/core/ir/var.cc
    lang/core/ir/var.h
    lang/core/ir/transform/add_empty_entry_point.cc
    lang/core/ir/transform/add_empty_entry_point.h
    lang/core/ir/transform/block_decorated_structs.cc
    lang/core/ir/transform/block_decorated_structs.h
    lang/core/ir/transform/demote_to_helper.cc
    lang/core/ir/transform/demote_to_helper.h
    lang/core/ir/transform/shader_io.cc
    lang/core/ir/transform/shader_io.h
    lang/core/ir/transform/transform.cc
    lang/core/ir/transform/transform.h
  )
endif()

if(MSVC)
  list(APPEND TINT_LIB_SRCS
    tint.natvis
  )
endif()

## Tint IO utilities. Used by tint_val.
add_library(tint_utils_io
  utils/command/command_${TINT_OS_CC_SUFFIX}.cc
  utils/command/command.h
  utils/file/tmpfile_${TINT_OS_CC_SUFFIX}.cc
  utils/file/tmpfile.h
)
tint_default_compile_options(tint_utils_io)
target_link_libraries(tint_utils_io tint_diagnostic_utils)

## Tint validation utilities. Used by tests and the tint executable.
add_library(tint_val
  lang/hlsl/validate/hlsl.cc
  lang/hlsl/validate/val.h
  lang/msl/validate/msl.cc
  lang/msl/validate/val.h
)

# If we're building on mac / ios and we have CoreGraphics, then we can use the
# metal API to validate our shaders. This is roughly 4x faster than invoking
# the metal shader compiler executable.
if(APPLE)
  find_library(LIB_CORE_GRAPHICS CoreGraphics)
  if(LIB_CORE_GRAPHICS)
    target_sources(tint_val PRIVATE "lang/msl/validate/msl_metal.mm")
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
    lang/core/builtin/number_test.cc
    lang/core/constant/composite_test.cc
    lang/core/constant/manager_test.cc
    lang/core/constant/scalar_test.cc
    lang/core/constant/splat_test.cc
    lang/core/constant/value_test.cc
    lang/core/type/array_test.cc
    lang/core/type/atomic_test.cc
    lang/core/type/bool_test.cc
    lang/core/type/builtin_structs_test.cc
    lang/core/type/depth_multisampled_texture_test.cc
    lang/core/type/depth_texture_test.cc
    lang/core/type/external_texture_test.cc
    lang/core/type/f16_test.cc
    lang/core/type/f32_test.cc
    lang/core/type/i32_test.cc
    lang/core/type/manager_test.cc
    lang/core/type/matrix_test.cc
    lang/core/type/multisampled_texture_test.cc
    lang/core/type/pointer_test.cc
    lang/core/type/reference_test.cc
    lang/core/type/sampled_texture_test.cc
    lang/core/type/sampler_test.cc
    lang/core/type/storage_texture_test.cc
    lang/core/type/struct_test.cc
    lang/core/type/texture_test.cc
    lang/core/type/type_test.cc
    lang/core/type/u32_test.cc
    lang/core/type/vector_test.cc
    lang/wgsl/ast/alias_test.cc
    lang/wgsl/ast/assignment_statement_test.cc
    lang/wgsl/ast/binary_expression_test.cc
    lang/wgsl/ast/binding_attribute_test.cc
    lang/wgsl/ast/bitcast_expression_test.cc
    lang/wgsl/ast/block_statement_test.cc
    lang/wgsl/ast/bool_literal_expression_test.cc
    lang/wgsl/ast/break_if_statement_test.cc
    lang/wgsl/ast/break_statement_test.cc
    lang/wgsl/ast/builtin_attribute_test.cc
    lang/wgsl/ast/builtin_texture_helper_test.cc
    lang/wgsl/ast/builtin_texture_helper_test.h
    lang/wgsl/ast/call_expression_test.cc
    lang/wgsl/ast/call_statement_test.cc
    lang/wgsl/ast/case_selector_test.cc
    lang/wgsl/ast/case_statement_test.cc
    lang/wgsl/ast/clone_context_test.cc
    lang/wgsl/ast/compound_assignment_statement_test.cc
    lang/wgsl/ast/const_assert_test.cc
    lang/wgsl/ast/continue_statement_test.cc
    lang/wgsl/ast/diagnostic_attribute_test.cc
    lang/wgsl/ast/diagnostic_control_test.cc
    lang/wgsl/ast/diagnostic_directive_test.cc
    lang/wgsl/ast/diagnostic_rule_name_test.cc
    lang/wgsl/ast/discard_statement_test.cc
    lang/wgsl/ast/enable_test.cc
    lang/wgsl/ast/float_literal_expression_test.cc
    lang/wgsl/ast/for_loop_statement_test.cc
    lang/wgsl/ast/function_test.cc
    lang/wgsl/ast/group_attribute_test.cc
    lang/wgsl/ast/id_attribute_test.cc
    lang/wgsl/ast/identifier_expression_test.cc
    lang/wgsl/ast/identifier_test.cc
    lang/wgsl/ast/if_statement_test.cc
    lang/wgsl/ast/increment_decrement_statement_test.cc
    lang/wgsl/ast/index_accessor_expression_test.cc
    lang/wgsl/ast/index_attribute_test.cc
    lang/wgsl/ast/int_literal_expression_test.cc
    lang/wgsl/ast/interpolate_attribute_test.cc
    lang/wgsl/ast/location_attribute_test.cc
    lang/wgsl/ast/loop_statement_test.cc
    lang/wgsl/ast/member_accessor_expression_test.cc
    lang/wgsl/ast/module_test.cc
    lang/wgsl/ast/phony_expression_test.cc
    lang/wgsl/ast/return_statement_test.cc
    lang/wgsl/ast/stage_attribute_test.cc
    lang/wgsl/ast/stride_attribute_test.cc
    lang/wgsl/ast/struct_member_align_attribute_test.cc
    lang/wgsl/ast/struct_member_offset_attribute_test.cc
    lang/wgsl/ast/struct_member_size_attribute_test.cc
    lang/wgsl/ast/struct_member_test.cc
    lang/wgsl/ast/struct_test.cc
    lang/wgsl/ast/switch_statement_test.cc
    lang/wgsl/ast/templated_identifier_test.cc
    lang/wgsl/ast/test_helper.h
    lang/wgsl/ast/test_helper_test.cc
    lang/wgsl/ast/transform/manager_test.cc
    lang/wgsl/ast/transform/transform_test.cc
    lang/wgsl/ast/traverse_expressions_test.cc
    lang/wgsl/ast/unary_op_expression_test.cc
    lang/wgsl/ast/variable_decl_statement_test.cc
    lang/wgsl/ast/variable_test.cc
    lang/wgsl/ast/while_statement_test.cc
    lang/wgsl/ast/workgroup_attribute_test.cc
    lang/wgsl/helpers/append_vector_test.cc
    lang/wgsl/helpers/check_supported_extensions_test.cc
    lang/wgsl/helpers/flatten_bindings_test.cc
    lang/wgsl/program/clone_context_test.cc
    lang/wgsl/program/program_builder_test.cc
    lang/wgsl/program/program_test.cc
    lang/wgsl/sem/builtin_test.cc
    lang/wgsl/sem/diagnostic_severity_test.cc
    lang/wgsl/sem/struct_test.cc
    lang/wgsl/sem/value_expression_test.cc
    lang/wgsl/resolver/address_space_layout_validation_test.cc
    lang/wgsl/resolver/address_space_validation_test.cc
    lang/wgsl/resolver/alias_analysis_test.cc
    lang/wgsl/resolver/array_accessor_test.cc
    lang/wgsl/resolver/assignment_validation_test.cc
    lang/wgsl/resolver/atomics_test.cc
    lang/wgsl/resolver/atomics_validation_test.cc
    lang/wgsl/resolver/attribute_validation_test.cc
    lang/wgsl/resolver/bitcast_validation_test.cc
    lang/wgsl/resolver/builtin_enum_test.cc
    lang/wgsl/resolver/builtin_structs_test.cc
    lang/wgsl/resolver/builtin_test.cc
    lang/wgsl/resolver/builtin_validation_test.cc
    lang/wgsl/resolver/builtins_validation_test.cc
    lang/wgsl/resolver/call_test.cc
    lang/wgsl/resolver/call_validation_test.cc
    lang/wgsl/resolver/compound_assignment_validation_test.cc
    lang/wgsl/resolver/compound_statement_test.cc
    lang/wgsl/resolver/const_assert_test.cc
    lang/wgsl/resolver/const_eval_binary_op_test.cc
    lang/wgsl/resolver/const_eval_bitcast_test.cc
    lang/wgsl/resolver/const_eval_builtin_test.cc
    lang/wgsl/resolver/const_eval_construction_test.cc
    lang/wgsl/resolver/const_eval_conversion_test.cc
    lang/wgsl/resolver/const_eval_indexing_test.cc
    lang/wgsl/resolver/const_eval_member_access_test.cc
    lang/wgsl/resolver/const_eval_runtime_semantics_test.cc
    lang/wgsl/resolver/const_eval_test.h
    lang/wgsl/resolver/const_eval_unary_op_test.cc
    lang/wgsl/resolver/control_block_validation_test.cc
    lang/wgsl/resolver/dependency_graph_test.cc
    lang/wgsl/resolver/diagnostic_control_test.cc
    lang/wgsl/resolver/dual_source_blending_extension_test.cc
    lang/wgsl/resolver/entry_point_validation_test.cc
    lang/wgsl/resolver/evaluation_stage_test.cc
    lang/wgsl/resolver/expression_kind_test.cc
    lang/wgsl/resolver/f16_extension_test.cc
    lang/wgsl/resolver/function_validation_test.cc
    lang/wgsl/resolver/host_shareable_validation_test.cc
    lang/wgsl/resolver/increment_decrement_validation_test.cc
    lang/wgsl/resolver/inferred_type_test.cc
    lang/wgsl/resolver/intrinsic_table_test.cc
    lang/wgsl/resolver/is_host_shareable_test.cc
    lang/wgsl/resolver/is_storeable_test.cc
    lang/wgsl/resolver/load_test.cc
    lang/wgsl/resolver/materialize_test.cc
    lang/wgsl/resolver/override_test.cc
    lang/wgsl/resolver/ptr_ref_test.cc
    lang/wgsl/resolver/ptr_ref_validation_test.cc
    lang/wgsl/resolver/resolver_behavior_test.cc
    lang/wgsl/resolver/resolver_test.cc
    lang/wgsl/resolver/resolver_test_helper.cc
    lang/wgsl/resolver/resolver_test_helper.h
    lang/wgsl/resolver/root_identifier_test.cc
    lang/wgsl/resolver/side_effects_test.cc
    lang/wgsl/resolver/struct_address_space_use_test.cc
    lang/wgsl/resolver/struct_layout_test.cc
    lang/wgsl/resolver/struct_pipeline_stage_use_test.cc
    lang/wgsl/resolver/type_validation_test.cc
    lang/wgsl/resolver/unresolved_identifier_test.cc
    lang/wgsl/resolver/validation_test.cc
    lang/wgsl/resolver/validator_is_storeable_test.cc
    lang/wgsl/resolver/value_constructor_validation_test.cc
    lang/wgsl/resolver/variable_test.cc
    lang/wgsl/resolver/variable_validation_test.cc
    test_main.cc
    utils/cli/cli_test.cc
    utils/command/command_test.cc
    utils/containers/bitset_test.cc
    utils/containers/enum_set_test.cc
    utils/containers/hashmap_test.cc
    utils/containers/hashset_test.cc
    utils/containers/map_test.cc
    utils/containers/predicates_test.cc
    utils/containers/reverse_test.cc
    utils/containers/scope_stack_test.cc
    utils/containers/slice_test.cc
    utils/containers/transform_test.cc
    utils/containers/unique_allocator_test.cc
    utils/containers/unique_vector_test.cc
    utils/containers/vector_test.cc
    utils/diagnostic/diagnostic_test.cc
    utils/diagnostic/formatter_test.cc
    utils/diagnostic/printer_test.cc
    utils/diagnostic/source_test.cc
    utils/file/tmpfile_test.cc
    utils/ice/ice_test.cc
    utils/macros/defer_test.cc
    utils/macros/scoped_assignment_test.cc
    utils/math/crc32_test.cc
    utils/math/hash_test.cc
    utils/math/math_test.cc
    utils/memory/bitcast_test.cc
    utils/memory/block_allocator_test.cc
    utils/memory/bump_allocator_test.cc
    utils/reflection/reflection_test.cc
    utils/result/result_test.cc
    utils/rtti/castable_test.cc
    utils/rtti/switch_test.cc
    utils/text/float_to_string_test.cc
    utils/text/string_stream_test.cc
    utils/text/string_test.cc
    utils/symbol/symbol_table_test.cc
    utils/symbol/symbol_test.cc
    utils/text/unicode_test.cc
    utils/traits/traits_test.cc
  )

  # Note, the source files are included here otherwise the cmd sources would not be included in the
  # test binary.
  list(APPEND TINT_TEST_SRCS
    cmd/generate_external_texture_bindings.cc
    cmd/generate_external_texture_bindings.h
    cmd/generate_external_texture_bindings_test.cc
  )

  # Uniformity analysis tests depend on WGSL reader
  if(${TINT_BUILD_WGSL_READER})
    list(APPEND TINT_TEST_SRCS
      lang/wgsl/resolver/uniformity_test.cc
    )
  endif()

  # Inspector tests depend on WGSL reader
  if(${TINT_BUILD_WGSL_READER})
    list(APPEND TINT_TEST_SRCS
      lang/wgsl/inspector/inspector_test.cc
      lang/wgsl/inspector/test_inspector_builder.cc
      lang/wgsl/inspector/test_inspector_builder.h
      lang/wgsl/inspector/test_inspector_runner.cc
      lang/wgsl/inspector/test_inspector_runner.h
    )
  endif()

  if(${TINT_BUILD_SPV_READER} AND ${TINT_BUILD_WGSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      lang/spirv/reader/ast_parser/ast_parser_test.cc
      lang/spirv/reader/ast_parser/barrier_test.cc
      lang/spirv/reader/ast_parser/constant_test.cc
      lang/spirv/reader/ast_parser/convert_member_decoration_test.cc
      lang/spirv/reader/ast_parser/convert_type_test.cc
      lang/spirv/reader/ast_parser/enum_converter_test.cc
      lang/spirv/reader/ast_parser/fail_stream_test.cc
      lang/spirv/reader/ast_parser/function_arithmetic_test.cc
      lang/spirv/reader/ast_parser/function_bit_test.cc
      lang/spirv/reader/ast_parser/function_call_test.cc
      lang/spirv/reader/ast_parser/function_cfg_test.cc
      lang/spirv/reader/ast_parser/function_composite_test.cc
      lang/spirv/reader/ast_parser/function_conversion_test.cc
      lang/spirv/reader/ast_parser/module_function_decl_test.cc
      lang/spirv/reader/ast_parser/function_decl_test.cc
      lang/spirv/reader/ast_parser/function_glsl_std_450_test.cc
      lang/spirv/reader/ast_parser/function_logical_test.cc
      lang/spirv/reader/ast_parser/function_memory_test.cc
      lang/spirv/reader/ast_parser/function_misc_test.cc
      lang/spirv/reader/ast_parser/function_var_test.cc
      lang/spirv/reader/ast_parser/get_decorations_test.cc
      lang/spirv/reader/ast_parser/handle_test.cc
      lang/spirv/reader/ast_parser/import_test.cc
      lang/spirv/reader/ast_parser/module_var_test.cc
      lang/spirv/reader/ast_parser/named_types_test.cc
      lang/spirv/reader/ast_parser/namer_test.cc
      lang/spirv/reader/ast_parser/parser_test.cc
      lang/spirv/reader/ast_parser/spirv_tools_helpers_test.cc
      lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h
      lang/spirv/reader/ast_parser/test_helper.cc
      lang/spirv/reader/ast_parser/test_helper.h
      lang/spirv/reader/ast_parser/type_test.cc
      lang/spirv/reader/ast_parser/usage_test.cc
      lang/spirv/reader/ast_parser/user_name_test.cc
    )
  endif()


  if(${TINT_BUILD_IR})
    list(APPEND TINT_TEST_SRCS
      lang/wgsl/helpers/ir_program_test.h
    )
  endif()

  if(${TINT_BUILD_WGSL_READER})
    list(APPEND TINT_TEST_SRCS
      lang/wgsl/reader/parser/additive_expression_test.cc
      lang/wgsl/reader/parser/argument_expression_list_test.cc
      lang/wgsl/reader/parser/assignment_stmt_test.cc
      lang/wgsl/reader/parser/bitwise_expression_test.cc
      lang/wgsl/reader/parser/break_stmt_test.cc
      lang/wgsl/reader/parser/bug_cases_test.cc
      lang/wgsl/reader/parser/call_stmt_test.cc
      lang/wgsl/reader/parser/classify_template_args_test.cc
      lang/wgsl/reader/parser/compound_stmt_test.cc
      lang/wgsl/reader/parser/const_literal_test.cc
      lang/wgsl/reader/parser/continue_stmt_test.cc
      lang/wgsl/reader/parser/continuing_stmt_test.cc
      lang/wgsl/reader/parser/core_lhs_expression_test.cc
      lang/wgsl/reader/parser/diagnostic_attribute_test.cc
      lang/wgsl/reader/parser/diagnostic_control_test.cc
      lang/wgsl/reader/parser/diagnostic_directive_test.cc
      lang/wgsl/reader/parser/enable_directive_test.cc
      lang/wgsl/reader/parser/error_msg_test.cc
      lang/wgsl/reader/parser/error_resync_test.cc
      lang/wgsl/reader/parser/expression_test.cc
      lang/wgsl/reader/parser/for_stmt_test.cc
      lang/wgsl/reader/parser/function_attribute_list_test.cc
      lang/wgsl/reader/parser/function_attribute_test.cc
      lang/wgsl/reader/parser/function_decl_test.cc
      lang/wgsl/reader/parser/function_header_test.cc
      lang/wgsl/reader/parser/global_constant_decl_test.cc
      lang/wgsl/reader/parser/global_decl_test.cc
      lang/wgsl/reader/parser/global_variable_decl_test.cc
      lang/wgsl/reader/parser/if_stmt_test.cc
      lang/wgsl/reader/parser/increment_decrement_stmt_test.cc
      lang/wgsl/reader/parser/lexer_test.cc
      lang/wgsl/reader/parser/lhs_expression_test.cc
      lang/wgsl/reader/parser/loop_stmt_test.cc
      lang/wgsl/reader/parser/math_expression_test.cc
      lang/wgsl/reader/parser/multiplicative_expression_test.cc
      lang/wgsl/reader/parser/param_list_test.cc
      lang/wgsl/reader/parser/paren_expression_test.cc
      lang/wgsl/reader/parser/parser_test.cc
      lang/wgsl/reader/parser/primary_expression_test.cc
      lang/wgsl/reader/parser/relational_expression_test.cc
      lang/wgsl/reader/parser/require_directive_test.cc
      lang/wgsl/reader/parser/reserved_keyword_test.cc
      lang/wgsl/reader/parser/shift_expression_test.cc
      lang/wgsl/reader/parser/singular_expression_test.cc
      lang/wgsl/reader/parser/statement_test.cc
      lang/wgsl/reader/parser/statements_test.cc
      lang/wgsl/reader/parser/struct_attribute_decl_test.cc
      lang/wgsl/reader/parser/struct_body_decl_test.cc
      lang/wgsl/reader/parser/struct_decl_test.cc
      lang/wgsl/reader/parser/struct_member_attribute_decl_test.cc
      lang/wgsl/reader/parser/struct_member_attribute_test.cc
      lang/wgsl/reader/parser/struct_member_test.cc
      lang/wgsl/reader/parser/switch_body_test.cc
      lang/wgsl/reader/parser/switch_stmt_test.cc
      lang/wgsl/reader/parser/test_helper.cc
      lang/wgsl/reader/parser/test_helper.h
      lang/wgsl/reader/parser/token_test.cc
      lang/wgsl/reader/parser/type_alias_test.cc
      lang/wgsl/reader/parser/type_decl_test.cc
      lang/wgsl/reader/parser/unary_expression_test.cc
      lang/wgsl/reader/parser/variable_attribute_list_test.cc
      lang/wgsl/reader/parser/variable_attribute_test.cc
      lang/wgsl/reader/parser/variable_decl_test.cc
      lang/wgsl/reader/parser/variable_ident_decl_test.cc
      lang/wgsl/reader/parser/variable_qualifier_test.cc
      lang/wgsl/reader/parser/variable_stmt_test.cc
      lang/wgsl/reader/parser/while_stmt_test.cc
    )

    if(${TINT_BUILD_IR})
      list(APPEND TINT_TEST_SRCS
        lang/wgsl/reader/program_to_ir/accessor_test.cc
        lang/wgsl/reader/program_to_ir/binary_test.cc
        lang/wgsl/reader/program_to_ir/builtin_test.cc
        lang/wgsl/reader/program_to_ir/call_test.cc
        lang/wgsl/reader/program_to_ir/function_test.cc
        lang/wgsl/reader/program_to_ir/let_test.cc
        lang/wgsl/reader/program_to_ir/literal_test.cc
        lang/wgsl/reader/program_to_ir/materialize_test.cc
        lang/wgsl/reader/program_to_ir/program_to_ir_test.cc
        lang/wgsl/reader/program_to_ir/shadowing_test.cc
        lang/wgsl/reader/program_to_ir/store_test.cc
        lang/wgsl/reader/program_to_ir/unary_test.cc
        lang/wgsl/reader/program_to_ir/var_test.cc
      )
    endif()
  endif()

  if(${TINT_BUILD_SPV_WRITER})
    list(APPEND TINT_TEST_SRCS
      lang/spirv/writer/ast_printer/accessor_expression_test.cc
      lang/spirv/writer/ast_printer/assign_test.cc
      lang/spirv/writer/ast_printer/ast_builtin_test.cc
      lang/spirv/writer/ast_printer/ast_discard_test.cc
      lang/spirv/writer/ast_printer/ast_function_test.cc
      lang/spirv/writer/ast_printer/ast_if_test.cc
      lang/spirv/writer/ast_printer/ast_loop_test.cc
      lang/spirv/writer/ast_printer/ast_printer_test.cc
      lang/spirv/writer/ast_printer/ast_switch_test.cc
      lang/spirv/writer/ast_printer/ast_type_test.cc
      lang/spirv/writer/ast_printer/binary_expression_test.cc
      lang/spirv/writer/ast_printer/bitcast_expression_test.cc
      lang/spirv/writer/ast_printer/block_test.cc
      lang/spirv/writer/ast_printer/builtin_texture_test.cc
      lang/spirv/writer/ast_printer/call_test.cc
      lang/spirv/writer/ast_printer/const_assert_test.cc
      lang/spirv/writer/ast_printer/constructor_expression_test.cc
      lang/spirv/writer/ast_printer/entry_point_test.cc
      lang/spirv/writer/ast_printer/format_conversion_test.cc
      lang/spirv/writer/ast_printer/function_attribute_test.cc
      lang/spirv/writer/ast_printer/function_variable_test.cc
      lang/spirv/writer/ast_printer/global_variable_test.cc
      lang/spirv/writer/ast_printer/ident_expression_test.cc
      lang/spirv/writer/ast_printer/literal_test.cc
      lang/spirv/writer/ast_printer/return_test.cc
      lang/spirv/writer/ast_printer/scalar_constant_test.cc
      lang/spirv/writer/ast_printer/test_helper.h
      lang/spirv/writer/ast_printer/unary_op_expression_test.cc
      lang/spirv/writer/binary_writer_test.cc
      lang/spirv/writer/instruction_test.cc
      lang/spirv/writer/module_test.cc
      lang/spirv/writer/operand_test.cc
      lang/spirv/writer/spv_dump.cc
      lang/spirv/writer/spv_dump.h
    )

    if(${TINT_BUILD_IR})
      list(APPEND TINT_TEST_SRCS
        lang/core/ir/transform/builtin_polyfill_spirv_test.cc
        lang/core/ir/transform/handle_matrix_arithmetic_test.cc
        lang/core/ir/transform/expand_implicit_splats_test.cc
        lang/core/ir/transform/merge_return_test.cc
        lang/core/ir/transform/shader_io_spirv_test.cc
        lang/core/ir/transform/std140_test.cc
        lang/core/ir/transform/var_for_dynamic_index_test.cc
        lang/spirv/writer/access_test.cc
        lang/spirv/writer/atomic_builtin_test.cc
        lang/spirv/writer/binary_test.cc
        lang/spirv/writer/bitcast_test.cc
        lang/spirv/writer/builtin_test.cc
        lang/spirv/writer/constant_test.cc
        lang/spirv/writer/construct_test.cc
        lang/spirv/writer/convert_test.cc
        lang/spirv/writer/discard_test.cc
        lang/spirv/writer/function_test.cc
        lang/spirv/writer/if_test.cc
        lang/spirv/writer/let_test.cc
        lang/spirv/writer/loop_test.cc
        lang/spirv/writer/switch_test.cc
        lang/spirv/writer/swizzle_test.cc
        lang/spirv/writer/test_helper.h
        lang/spirv/writer/texture_builtin_test.cc
        lang/spirv/writer/type_test.cc
        lang/spirv/writer/unary_test.cc
        lang/spirv/writer/var_test.cc
        lang/spirv/writer/writer_test.cc
      )
    endif()
  endif()

  if(${TINT_BUILD_WGSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      lang/wgsl/writer/ast_printer/ast_printer_test.cc
      lang/wgsl/writer/ast_printer/alias_type_test.cc
      lang/wgsl/writer/ast_printer/array_accessor_test.cc
      lang/wgsl/writer/ast_printer/assign_test.cc
      lang/wgsl/writer/ast_printer/binary_test.cc
      lang/wgsl/writer/ast_printer/bitcast_test.cc
      lang/wgsl/writer/ast_printer/block_test.cc
      lang/wgsl/writer/ast_printer/break_test.cc
      lang/wgsl/writer/ast_printer/call_test.cc
      lang/wgsl/writer/ast_printer/case_test.cc
      lang/wgsl/writer/ast_printer/cast_test.cc
      lang/wgsl/writer/ast_printer/const_assert_test.cc
      lang/wgsl/writer/ast_printer/constructor_test.cc
      lang/wgsl/writer/ast_printer/continue_test.cc
      lang/wgsl/writer/ast_printer/diagnostic_test.cc
      lang/wgsl/writer/ast_printer/discard_test.cc
      lang/wgsl/writer/ast_printer/enable_test.cc
      lang/wgsl/writer/ast_printer/function_test.cc
      lang/wgsl/writer/ast_printer/global_decl_test.cc
      lang/wgsl/writer/ast_printer/identifier_test.cc
      lang/wgsl/writer/ast_printer/if_test.cc
      lang/wgsl/writer/ast_printer/loop_test.cc
      lang/wgsl/writer/ast_printer/literal_test.cc
      lang/wgsl/writer/ast_printer/member_accessor_test.cc
      lang/wgsl/writer/ast_printer/return_test.cc
      lang/wgsl/writer/ast_printer/switch_test.cc
      lang/wgsl/writer/ast_printer/type_test.cc
      lang/wgsl/writer/ast_printer/unary_op_test.cc
      lang/wgsl/writer/ast_printer/variable_decl_statement_test.cc
      lang/wgsl/writer/ast_printer/variable_test.cc
      lang/wgsl/writer/ast_printer/test_helper.h
    )

    if (${TINT_BUILD_IR})
      list(APPEND TINT_TEST_SRCS
        lang/wgsl/writer/ir_to_program/inlining_test.cc
        lang/wgsl/writer/ir_to_program/ir_to_program_test.cc
        lang/wgsl/writer/ir_to_program/rename_conflicts_test.cc
      )
    endif()
  endif()



  if(${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_WGSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      lang/wgsl/ast/module_clone_test.cc
      lang/wgsl/ast/transform/add_empty_entry_point_test.cc
      lang/wgsl/ast/transform/add_block_attribute_test.cc
      lang/wgsl/ast/transform/array_length_from_uniform_test.cc
      lang/wgsl/ast/transform/binding_remapper_test.cc
      lang/wgsl/ast/transform/builtin_polyfill_test.cc
      lang/wgsl/ast/transform/calculate_array_length_test.cc
      lang/wgsl/ast/transform/clamp_frag_depth_test.cc
      lang/wgsl/ast/transform/canonicalize_entry_point_io_test.cc
      lang/wgsl/ast/transform/combine_samplers_test.cc
      lang/wgsl/ast/transform/decompose_memory_access_test.cc
      lang/wgsl/ast/transform/decompose_strided_array_test.cc
      lang/wgsl/ast/transform/decompose_strided_matrix_test.cc
      lang/wgsl/ast/transform/demote_to_helper_test.cc
      lang/wgsl/ast/transform/direct_variable_access_test.cc
      lang/wgsl/ast/transform/disable_uniformity_analysis_test.cc
      lang/wgsl/ast/transform/expand_compound_assignment_test.cc
      lang/wgsl/ast/transform/first_index_offset_test.cc
      lang/wgsl/ast/transform/fold_trivial_lets_test.cc
      lang/wgsl/ast/transform/for_loop_to_loop_test.cc
      lang/wgsl/ast/transform/expand_compound_assignment_test.cc
      lang/wgsl/ast/transform/localize_struct_array_assignment_test.cc
      lang/wgsl/ast/transform/merge_return_test.cc
      lang/wgsl/ast/transform/module_scope_var_to_entry_point_param_test.cc
      lang/wgsl/ast/transform/multiplanar_external_texture_test.cc
      lang/wgsl/ast/transform/num_workgroups_from_uniform_test.cc
      lang/wgsl/ast/transform/packed_vec3_test.cc
      lang/wgsl/ast/transform/pad_structs_test.cc
      lang/wgsl/ast/transform/preserve_padding_test.cc
      lang/wgsl/ast/transform/promote_initializers_to_let_test.cc
      lang/wgsl/ast/transform/promote_side_effects_to_decl_test.cc
      lang/wgsl/ast/transform/remove_continue_in_switch_test.cc
      lang/wgsl/ast/transform/remove_phonies_test.cc
      lang/wgsl/ast/transform/remove_unreachable_statements_test.cc
      lang/wgsl/ast/transform/renamer_test.cc
      lang/wgsl/ast/transform/robustness_test.cc
      lang/wgsl/ast/transform/simplify_pointers_test.cc
      lang/wgsl/ast/transform/single_entry_point_test.cc
      lang/wgsl/ast/transform/spirv_atomic_test.cc
      lang/wgsl/ast/transform/std140_exhaustive_test.cc
      lang/wgsl/ast/transform/std140_f16_test.cc
      lang/wgsl/ast/transform/std140_f32_test.cc
      lang/wgsl/ast/transform/std140_test.cc
      lang/wgsl/ast/transform/substitute_override_test.cc
      lang/wgsl/ast/transform/test_helper.h
      lang/wgsl/ast/transform/texture_1d_to_2d_test.cc
      lang/wgsl/ast/transform/truncate_interstage_variables_test.cc
      lang/wgsl/ast/transform/unshadow_test.cc
      lang/wgsl/ast/transform/var_for_dynamic_index_test.cc
      lang/wgsl/ast/transform/vectorize_matrix_conversions_test.cc
      lang/wgsl/ast/transform/vectorize_scalar_matrix_initializers_test.cc
      lang/wgsl/ast/transform/vertex_pulling_test.cc
      lang/wgsl/ast/transform/while_to_loop_test.cc
      lang/wgsl/ast/transform/zero_init_workgroup_memory_test.cc
      lang/wgsl/ast/transform/utils/get_insertion_point_test.cc
      lang/wgsl/ast/transform/utils/hoist_to_decl_before_test.cc
    )
  endif()

  if(${TINT_BUILD_MSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      lang/msl/writer/ast_printer/array_accessor_test.cc
      lang/msl/writer/ast_printer/assign_test.cc
      lang/msl/writer/ast_printer/ast_function_test.cc
      lang/msl/writer/ast_printer/ast_printer_test.cc
      lang/msl/writer/ast_printer/ast_type_test.cc
      lang/msl/writer/ast_printer/binary_test.cc
      lang/msl/writer/ast_printer/bitcast_test.cc
      lang/msl/writer/ast_printer/block_test.cc
      lang/msl/writer/ast_printer/break_test.cc
      lang/msl/writer/ast_printer/builtin_test.cc
      lang/msl/writer/ast_printer/builtin_texture_test.cc
      lang/msl/writer/ast_printer/call_test.cc
      lang/msl/writer/ast_printer/case_test.cc
      lang/msl/writer/ast_printer/cast_test.cc
      lang/msl/writer/ast_printer/const_assert_test.cc
      lang/msl/writer/ast_printer/constructor_test.cc
      lang/msl/writer/ast_printer/continue_test.cc
      lang/msl/writer/ast_printer/discard_test.cc
      lang/msl/writer/ast_printer/identifier_test.cc
      lang/msl/writer/ast_printer/if_test.cc
      lang/msl/writer/ast_printer/import_test.cc
      lang/msl/writer/ast_printer/loop_test.cc
      lang/msl/writer/ast_printer/member_accessor_test.cc
      lang/msl/writer/ast_printer/module_constant_test.cc
      lang/msl/writer/ast_printer/return_test.cc
      lang/msl/writer/ast_printer/sanitizer_test.cc
      lang/msl/writer/ast_printer/switch_test.cc
      lang/msl/writer/ast_printer/test_helper.h
      lang/msl/writer/ast_printer/unary_op_test.cc
      lang/msl/writer/ast_printer/variable_decl_statement_test.cc
      lang/msl/writer/printer_support_test.cc
    )

    if(${TINT_BUILD_IR})
      list(APPEND TINT_TEST_SRCS
        lang/msl/writer/printer/constant_test.cc
        lang/msl/writer/printer/function_test.cc
        lang/msl/writer/printer/type_test.cc
        lang/msl/writer/printer/test_helper.h
      )
    endif()
  endif()

  if (${TINT_BUILD_GLSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      lang/glsl/writer/ast_printer/array_accessor_test.cc
      lang/glsl/writer/ast_printer/assign_test.cc
      lang/glsl/writer/ast_printer/ast_printer_test.cc
      lang/glsl/writer/ast_printer/binary_test.cc
      lang/glsl/writer/ast_printer/bitcast_test.cc
      lang/glsl/writer/ast_printer/block_test.cc
      lang/glsl/writer/ast_printer/break_test.cc
      lang/glsl/writer/ast_printer/builtin_test.cc
      lang/glsl/writer/ast_printer/builtin_texture_test.cc
      lang/glsl/writer/ast_printer/call_test.cc
      lang/glsl/writer/ast_printer/case_test.cc
      lang/glsl/writer/ast_printer/cast_test.cc
      lang/glsl/writer/ast_printer/constructor_test.cc
      lang/glsl/writer/ast_printer/continue_test.cc
      lang/glsl/writer/ast_printer/discard_test.cc
      lang/glsl/writer/ast_printer/function_test.cc
      lang/glsl/writer/ast_printer/identifier_test.cc
      lang/glsl/writer/ast_printer/if_test.cc
      lang/glsl/writer/ast_printer/import_test.cc
      lang/glsl/writer/ast_printer/loop_test.cc
      lang/glsl/writer/ast_printer/member_accessor_test.cc
      lang/glsl/writer/ast_printer/module_constant_test.cc
      lang/glsl/writer/ast_printer/return_test.cc
      lang/glsl/writer/ast_printer/sanitizer_test.cc
      lang/glsl/writer/ast_printer/storage_buffer_test.cc
      lang/glsl/writer/ast_printer/switch_test.cc
      lang/glsl/writer/ast_printer/type_test.cc
      lang/glsl/writer/ast_printer/unary_op_test.cc
      lang/glsl/writer/ast_printer/uniform_buffer_test.cc
      lang/glsl/writer/ast_printer/variable_decl_statement_test.cc
      lang/glsl/writer/ast_printer/workgroup_var_test.cc
      lang/glsl/writer/ast_printer/test_helper.h
    )
  endif()

  if (${TINT_BUILD_HLSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      lang/hlsl/writer/ast_printer/array_accessor_test.cc
      lang/hlsl/writer/ast_printer/assign_test.cc
      lang/hlsl/writer/ast_printer/ast_printer_test.cc
      lang/hlsl/writer/ast_printer/binary_test.cc
      lang/hlsl/writer/ast_printer/bitcast_test.cc
      lang/hlsl/writer/ast_printer/block_test.cc
      lang/hlsl/writer/ast_printer/break_test.cc
      lang/hlsl/writer/ast_printer/builtin_test.cc
      lang/hlsl/writer/ast_printer/builtin_texture_test.cc
      lang/hlsl/writer/ast_printer/call_test.cc
      lang/hlsl/writer/ast_printer/case_test.cc
      lang/hlsl/writer/ast_printer/cast_test.cc
      lang/hlsl/writer/ast_printer/const_assert_test.cc
      lang/hlsl/writer/ast_printer/constructor_test.cc
      lang/hlsl/writer/ast_printer/continue_test.cc
      lang/hlsl/writer/ast_printer/discard_test.cc
      lang/hlsl/writer/ast_printer/function_test.cc
      lang/hlsl/writer/ast_printer/identifier_test.cc
      lang/hlsl/writer/ast_printer/if_test.cc
      lang/hlsl/writer/ast_printer/import_test.cc
      lang/hlsl/writer/ast_printer/loop_test.cc
      lang/hlsl/writer/ast_printer/member_accessor_test.cc
      lang/hlsl/writer/ast_printer/module_constant_test.cc
      lang/hlsl/writer/ast_printer/return_test.cc
      lang/hlsl/writer/ast_printer/sanitizer_test.cc
      lang/hlsl/writer/ast_printer/switch_test.cc
      lang/hlsl/writer/ast_printer/type_test.cc
      lang/hlsl/writer/ast_printer/unary_op_test.cc
      lang/hlsl/writer/ast_printer/variable_decl_statement_test.cc
      lang/hlsl/writer/ast_printer/workgroup_var_test.cc
      lang/hlsl/writer/ast_printer/test_helper.h
    )
  endif()

  if (${TINT_BUILD_IR})
    list(APPEND TINT_TEST_SRCS
      lang/core/ir/access_test.cc
      lang/core/ir/binary_test.cc
      lang/core/ir/bitcast_test.cc
      lang/core/ir/block_param_test.cc
      lang/core/ir/block_test.cc
      lang/core/ir/break_if_test.cc
      lang/core/ir/constant_test.cc
      lang/core/ir/construct_test.cc
      lang/core/ir/continue_test.cc
      lang/core/ir/convert_test.cc
      lang/core/ir/core_builtin_call_test.cc
      lang/core/ir/discard_test.cc
      lang/core/ir/exit_if_test.cc
      lang/core/ir/exit_loop_test.cc
      lang/core/ir/exit_switch_test.cc
      lang/core/ir/function_param_test.cc
      lang/core/ir/function_test.cc
      lang/core/ir/if_test.cc
      lang/core/ir/instruction_result_test.cc
      lang/core/ir/instruction_test.cc
      lang/core/ir/intrinsic_call_test.cc
      lang/core/ir/let_test.cc
      lang/core/ir/load_test.cc
      lang/core/ir/load_vector_element_test.cc
      lang/core/ir/loop_test.cc
      lang/core/ir/module_test.cc
      lang/core/ir/multi_in_block_test.cc
      lang/core/ir/next_iteration_test.cc
      lang/core/ir/operand_instruction_test.cc
      lang/core/ir/return_test.cc
      lang/core/ir/store_test.cc
      lang/core/ir/store_vector_element_test.cc
      lang/core/ir/switch_test.cc
      lang/core/ir/swizzle_test.cc
      lang/core/ir/transform/add_empty_entry_point_test.cc
      lang/core/ir/transform/block_decorated_structs_test.cc
      lang/core/ir/transform/demote_to_helper_test.cc
      lang/core/ir/unary_test.cc
      lang/core/ir/user_call_test.cc
      lang/core/ir/validator_test.cc
      lang/core/ir/value_test.cc
      lang/core/ir/var_test.cc
    )
  endif()

  if (${TINT_BUILD_IR} AND ${TINT_BUILD_WGSL_READER} AND ${TINT_BUILD_WGSL_WRITER})
    list(APPEND TINT_TEST_SRCS
      lang/wgsl/ir_roundtrip_test.cc
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
    "utils/rtti/switch_bench.cc"
    "bench/benchmark.cc"
    "lang/wgsl/reader/reader_bench.cc"
  )

  if (${TINT_BUILD_GLSL_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS lang/glsl/writer/writer_bench.cc)
  endif()
  if (${TINT_BUILD_HLSL_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS lang/hlsl/writer/writer_bench.cc)
  endif()
  if (${TINT_BUILD_MSL_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS lang/msl/writer/writer_bench.cc)
  endif()
  if (${TINT_BUILD_SPV_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS lang/spirv/writer/writer_bench.cc)
  endif()
  if (${TINT_BUILD_WGSL_WRITER})
    list(APPEND TINT_BENCHMARK_SRCS lang/wgsl/writer/writer_bench.cc)
  endif()

  add_executable(tint-benchmark ${TINT_BENCHMARK_SRCS})
  set_target_properties(${target} PROPERTIES FOLDER "Benchmarks")

  tint_core_compile_options(tint-benchmark)

  target_link_libraries(tint-benchmark PRIVATE benchmark::benchmark libtint)

  if (TINT_EXTERNAL_BENCHMARK_CORPUS_DIR)
    # Glob all the files at TINT_EXTERNAL_BENCHMARK_CORPUS_DIR, and create a header
    # that lists these with the macros:
    # TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS()
    # TINT_BENCHMARK_EXTERNAL_SPV_PROGRAMS()
    set(TINT_BENCHMARK_GEN_DIR "${DAWN_BUILD_GEN_DIR}/src/tint/benchmark/")
    set(TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER "${TINT_BENCHMARK_GEN_DIR}/external_wgsl_programs.h")
    message("Globbing ${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}...")

    file(GLOB_RECURSE
      TINT_EXTERNAL_WGSL_BENCHMARK_FILES
      RELATIVE "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}"
      "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/**.wgsl")
    list(TRANSFORM TINT_EXTERNAL_WGSL_BENCHMARK_FILES REPLACE
      "(.+)"
      "    BENCHMARK_CAPTURE\(FUNC, \"\\1\", \"${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/\\1\")")
    list(JOIN TINT_EXTERNAL_WGSL_BENCHMARK_FILES "; \\\n" TINT_EXTERNAL_WGSL_BENCHMARK_FILES)

    file(GLOB_RECURSE
      TINT_EXTERNAL_SPV_BENCHMARK_FILES
      RELATIVE "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}"
      "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/**.spv")

      list(TRANSFORM TINT_EXTERNAL_SPV_BENCHMARK_FILES REPLACE
      "(.+)"
      "    BENCHMARK_CAPTURE\(FUNC, \"\\1\", \"${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/\\1\")")
    list(JOIN TINT_EXTERNAL_SPV_BENCHMARK_FILES "; \\\n" TINT_EXTERNAL_SPV_BENCHMARK_FILES)

    file(CONFIGURE
      OUTPUT "${TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER}"
      CONTENT "
#define TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS(FUNC) \\
${TINT_EXTERNAL_WGSL_BENCHMARK_FILES};

#define TINT_BENCHMARK_EXTERNAL_SPV_PROGRAMS(FUNC) \\
${TINT_EXTERNAL_SPV_BENCHMARK_FILES};")
    # Define TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER to the generated header path
    target_compile_definitions(tint-benchmark PRIVATE
      "TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER=\"${TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER}\"")
  endif()
endif(TINT_BUILD_BENCHMARKS)
