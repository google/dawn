# Copyright 2026 The Dawn & Tint Authors
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

if(NOT UNIX OR APPLE)
    message(FATAL_ERROR "Mesa Lavapipe build is currently only supported on Linux")
endif()

enable_language(ASM)

# Do not inherit root directory compile/link options (such as -Wno-c++20-compat
# which breaks ASM compilation, or -stdlib=libc++ which conflicts with the
# prebuilt libstdc++-based third_party/llvm-dev archives).
set_directory_properties(PROPERTIES COMPILE_OPTIONS "" LINK_OPTIONS "")

set(MESA_LLVM_DEV_DIR "${DAWN_THIRD_PARTY_DIR}/llvm-dev" CACHE PATH "Path to llvm-dev prebuilts for Mesa")
set(MESA_ZSTD_DIR "${DAWN_THIRD_PARTY_DIR}/zstd" CACHE PATH "Path to zstd prebuilts for Mesa")

include("${CMAKE_CURRENT_SOURCE_DIR}/mesa_sources.cmake")

find_package(Python3 REQUIRED)

set(_mesa_src_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(_mesa_gen_dir "${CMAKE_CURRENT_BINARY_DIR}")
set(_vpython_spec "${PROJECT_SOURCE_DIR}/vpython.toml")
set(_mesa_action_script "${_mesa_src_dir}/mesa_action.py")
set(_mesa_icd_json "${CMAKE_BINARY_DIR}/lvp_icd.json")

# Ensure all output directories for generated files exist prior to running
# Mesa's Python codegen scripts.
file(MAKE_DIRECTORY
    "${_mesa_gen_dir}/src"
    "${_mesa_gen_dir}/src/compiler"
    "${_mesa_gen_dir}/src/compiler/nir"
    "${_mesa_gen_dir}/src/compiler/spirv"
    "${_mesa_gen_dir}/src/gallium/auxiliary"
    "${_mesa_gen_dir}/src/gallium/frontends/lavapipe"
    "${_mesa_gen_dir}/src/util"
    "${_mesa_gen_dir}/src/util/format"
    "${_mesa_gen_dir}/src/vulkan/runtime"
    "${_mesa_gen_dir}/src/vulkan/runtime/bvh"
    "${_mesa_gen_dir}/src/vulkan/util"
    "${_mesa_gen_dir}/src/vulkan/wsi"
)

add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/libzstd.so.1"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MESA_ZSTD_DIR}/lib/libzstd.so.1"
        "${CMAKE_BINARY_DIR}/libzstd.so.1"
    DEPENDS "${MESA_ZSTD_DIR}/lib/libzstd.so.1"
    VERBATIM
)
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/libzstd.so.1.5.7"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MESA_ZSTD_DIR}/lib/libzstd.so.1.5.7"
        "${CMAKE_BINARY_DIR}/libzstd.so.1.5.7"
    DEPENDS "${MESA_ZSTD_DIR}/lib/libzstd.so.1.5.7"
    VERBATIM
)
add_custom_target(mesa_copy_zstd DEPENDS
    "${CMAKE_BINARY_DIR}/libzstd.so.1"
    "${CMAKE_BINARY_DIR}/libzstd.so.1.5.7"
)

# ---------------------------------------------------------------------------
# Codegen Actions (49 targets total, mirroring third_party/mesa/BUILD.gn)
# ---------------------------------------------------------------------------

set(_mesa_all_codegen_outputs "")

function(_mesa_action target_name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG
        "CAPTURE_STDOUT"
        "PYTHON_SCRIPT"
        "INPUTS;OUTPUTS;ARGS;DEPENDS"
    )
    set(_cmd_args
        "--vpython-spec" "${_vpython_spec}"
    )
    if(ARG_CAPTURE_STDOUT)
        list(GET ARG_OUTPUTS 0 _first_out)
        list(APPEND _cmd_args "--capture-stdout" "${_first_out}")
    endif()
    list(APPEND _cmd_args "--" "${ARG_PYTHON_SCRIPT}" ${ARG_ARGS})

    add_custom_command(
        OUTPUT ${ARG_OUTPUTS}
        COMMAND ${Python3_EXECUTABLE} "${_mesa_action_script}" ${_cmd_args}
        DEPENDS
            "${_mesa_action_script}"
            "${_vpython_spec}"
            "${ARG_PYTHON_SCRIPT}"
            ${ARG_INPUTS}
            ${ARG_DEPENDS}
        COMMENT "Generating ${target_name}"
        VERBATIM
    )

    set(_outputs "${_mesa_all_codegen_outputs}")
    list(APPEND _outputs ${ARG_OUTPUTS})
    set(_mesa_all_codegen_outputs "${_outputs}" PARENT_SCOPE)
endfunction()

_mesa_action(mesa_git_sha1
    PYTHON_SCRIPT "${_mesa_src_dir}/src/bin/git_sha1_gen.py"
    OUTPUTS "${_mesa_gen_dir}/src/git_sha1.h"
    ARGS
        "--output"
        "${_mesa_gen_dir}/src/git_sha1.h"
)

_mesa_action(mesa_driconf_static_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/util/driconf_static.py"
    INPUTS "${_mesa_src_dir}/src/src/util/00-mesa-defaults.conf"
    OUTPUTS "${_mesa_gen_dir}/src/util/driconf_static.h"
    ARGS
        "${_mesa_src_dir}/src/src/util/00-mesa-defaults.conf"
        "${_mesa_gen_dir}/src/util/driconf_static.h"
)

_mesa_action(mesa_format_srgb_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/util/format_srgb.py"
    OUTPUTS "${_mesa_gen_dir}/src/util/format_srgb.c"
    CAPTURE_STDOUT
)

_mesa_action(mesa_shader_stats_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/util/process_shader_stats.py"
    INPUTS
        "${_mesa_src_dir}/src/src/util/shader_stats.rnc"
        "${_mesa_src_dir}/src/src/util/shader_stats.xml"
    OUTPUTS "${_mesa_gen_dir}/src/util/shader_stats.h"
    CAPTURE_STDOUT
    ARGS
        "${_mesa_src_dir}/src/src/util/shader_stats.rnc"
        "${_mesa_src_dir}/src/src/util/shader_stats.xml"
)

set(_u_format_inputs
    "${_mesa_src_dir}/src/src/util/format/u_format.yaml"
    "${_mesa_src_dir}/src/src/util/format/u_format_pack.py"
    "${_mesa_src_dir}/src/src/util/format/u_format_parse.py"
)

_mesa_action(mesa_u_format_gen_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/util/format/u_format_table.py"
    INPUTS ${_u_format_inputs}
    OUTPUTS "${_mesa_gen_dir}/src/util/format/u_format_gen.h"
    CAPTURE_STDOUT
    ARGS
        "${_mesa_src_dir}/src/src/util/format/u_format.yaml"
        "--enums"
)

_mesa_action(mesa_u_format_pack_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/util/format/u_format_table.py"
    INPUTS ${_u_format_inputs}
    OUTPUTS "${_mesa_gen_dir}/src/util/format/u_format_pack.h"
    CAPTURE_STDOUT
    ARGS
        "${_mesa_src_dir}/src/src/util/format/u_format.yaml"
        "--header"
)

_mesa_action(mesa_u_format_table_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/util/format/u_format_table.py"
    INPUTS ${_u_format_inputs}
    OUTPUTS "${_mesa_gen_dir}/src/util/format/u_format_table.c"
    CAPTURE_STDOUT
    ARGS "${_mesa_src_dir}/src/src/util/format/u_format.yaml"
)

_mesa_action(mesa_builtin_types_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/builtin_types_h.py"
    INPUTS "${_mesa_src_dir}/src/src/compiler/builtin_types.py"
    OUTPUTS "${_mesa_gen_dir}/src/compiler/builtin_types.h"
    ARGS "${_mesa_gen_dir}/src/compiler/builtin_types.h"
)

_mesa_action(mesa_builtin_types_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/builtin_types_c.py"
    INPUTS "${_mesa_src_dir}/src/src/compiler/builtin_types.py"
    OUTPUTS "${_mesa_gen_dir}/src/compiler/builtin_types.c"
    ARGS "${_mesa_gen_dir}/src/compiler/builtin_types.c"
)

_mesa_action(mesa_ir_expression_operation_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/glsl/ir_expression_operation.py"
    OUTPUTS "${_mesa_gen_dir}/src/compiler/ir_expression_operation.h"
    CAPTURE_STDOUT
    ARGS "enum"
)

set(_nir_py_inputs
    "${_mesa_src_dir}/src/src/compiler/nir/nir_intrinsics.py"
    "${_mesa_src_dir}/src/src/compiler/nir/nir_opcodes.py"
)

_mesa_action(mesa_nir_builder_opcodes_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/nir/nir_builder_opcodes_h.py"
    INPUTS ${_nir_py_inputs}
    OUTPUTS "${_mesa_gen_dir}/src/compiler/nir/nir_builder_opcodes.h"
    CAPTURE_STDOUT
)

_mesa_action(mesa_nir_constant_expressions_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/nir/nir_constant_expressions.py"
    INPUTS ${_nir_py_inputs}
    OUTPUTS "${_mesa_gen_dir}/src/compiler/nir/nir_constant_expressions.c"
    CAPTURE_STDOUT
)

_mesa_action(mesa_nir_opcodes_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/nir/nir_opcodes_h.py"
    INPUTS ${_nir_py_inputs}
    OUTPUTS "${_mesa_gen_dir}/src/compiler/nir/nir_opcodes.h"
    CAPTURE_STDOUT
)

_mesa_action(mesa_nir_opcodes_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/nir/nir_opcodes_c.py"
    INPUTS ${_nir_py_inputs}
    OUTPUTS "${_mesa_gen_dir}/src/compiler/nir/nir_opcodes.c"
    CAPTURE_STDOUT
)

_mesa_action(mesa_nir_opt_algebraic_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/nir/nir_opt_algebraic.py"
    INPUTS
        ${_nir_py_inputs}
        "${_mesa_src_dir}/src/src/compiler/nir/nir_algebraic.py"
    OUTPUTS "${_mesa_gen_dir}/src/compiler/nir/nir_opt_algebraic.c"
    ARGS
        "--out"
        "${_mesa_gen_dir}/src/compiler/nir/nir_opt_algebraic.c"
)

_mesa_action(mesa_nir_intrinsics_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/nir/nir_intrinsics_h.py"
    INPUTS "${_mesa_src_dir}/src/src/compiler/nir/nir_intrinsics.py"
    OUTPUTS "${_mesa_gen_dir}/src/compiler/nir/nir_intrinsics.h"
    ARGS
        "--out"
        "${_mesa_gen_dir}/src/compiler/nir/nir_intrinsics.h"
)

_mesa_action(mesa_nir_intrinsics_indices_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/nir/nir_intrinsics_indices_h.py"
    INPUTS "${_mesa_src_dir}/src/src/compiler/nir/nir_intrinsics.py"
    OUTPUTS "${_mesa_gen_dir}/src/compiler/nir/nir_intrinsics_indices.h"
    ARGS
        "--out"
        "${_mesa_gen_dir}/src/compiler/nir/nir_intrinsics_indices.h"
)

_mesa_action(mesa_nir_intrinsics_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/nir/nir_intrinsics_c.py"
    INPUTS "${_mesa_src_dir}/src/src/compiler/nir/nir_intrinsics.py"
    OUTPUTS "${_mesa_gen_dir}/src/compiler/nir/nir_intrinsics.c"
    ARGS
        "--out"
        "${_mesa_gen_dir}/src/compiler/nir/nir_intrinsics.c"
)

_mesa_action(mesa_vtn_gather_types_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/spirv/vtn_gather_types_c.py"
    INPUTS "${_mesa_src_dir}/src/src/compiler/spirv/spirv.core.grammar.json"
    OUTPUTS "${_mesa_gen_dir}/src/compiler/spirv/vtn_gather_types.c"
    ARGS
        "${_mesa_src_dir}/src/src/compiler/spirv/spirv.core.grammar.json"
        "${_mesa_gen_dir}/src/compiler/spirv/vtn_gather_types.c"
)

_mesa_action(mesa_spirv_info
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/spirv/spirv_info_gen.py"
    INPUTS "${_mesa_src_dir}/src/src/compiler/spirv/spirv.core.grammar.json"
    OUTPUTS
        "${_mesa_gen_dir}/src/compiler/spirv/spirv_info.h"
        "${_mesa_gen_dir}/src/compiler/spirv/spirv_info.c"
    ARGS
        "--json"
        "${_mesa_src_dir}/src/src/compiler/spirv/spirv.core.grammar.json"
        "--out-h"
        "${_mesa_gen_dir}/src/compiler/spirv/spirv_info.h"
        "--out-c"
        "${_mesa_gen_dir}/src/compiler/spirv/spirv_info.c"
)

_mesa_action(mesa_vtn_generator_ids_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/compiler/spirv/vtn_generator_ids_h.py"
    INPUTS "${_mesa_src_dir}/src/src/compiler/spirv/spir-v.xml"
    OUTPUTS "${_mesa_gen_dir}/src/compiler/spirv/vtn_generator_ids.h"
    ARGS
        "${_mesa_src_dir}/src/src/compiler/spirv/spir-v.xml"
        "${_mesa_gen_dir}/src/compiler/spirv/vtn_generator_ids.h"
)

set(_vk_xml "${_mesa_src_dir}/src/src/vulkan/registry/vk.xml")
set(_vk_py_inputs
    "${_vk_xml}"
    "${_mesa_src_dir}/src/src/vulkan/util/vk_entrypoints.py"
    "${_mesa_src_dir}/src/src/vulkan/util/vk_extensions.py"
)

_mesa_action(mesa_vk_dispatch_table
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_dispatch_table_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/util/vk_dispatch_table.c"
        "${_mesa_gen_dir}/src/vulkan/util/vk_dispatch_table.h"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/util/vk_dispatch_table.c"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/util/vk_dispatch_table.h"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_enum_to_str
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/gen_enum_to_str.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/util/vk_enum_to_str.c"
        "${_mesa_gen_dir}/src/vulkan/util/vk_enum_to_str.h"
        "${_mesa_gen_dir}/src/vulkan/util/vk_enum_defines.h"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/util/vk_enum_to_str.c"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/util/vk_enum_to_str.h"
        "--out-d"
        "${_mesa_gen_dir}/src/vulkan/util/vk_enum_defines.h"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_struct_type_cast
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_struct_type_cast_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS "${_mesa_gen_dir}/src/vulkan/util/vk_struct_type_cast.h"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out"
        "${_mesa_gen_dir}/src/vulkan/util/vk_struct_type_cast.h"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_extensions
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_extensions_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/util/vk_extensions.c"
        "${_mesa_gen_dir}/src/vulkan/util/vk_extensions.h"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/util/vk_extensions.c"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/util/vk_extensions.h"
)

_mesa_action(mesa_vk_common_entrypoints
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_entrypoints_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_common_entrypoints.h"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_common_entrypoints.c"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--proto"
        "--weak"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_common_entrypoints.h"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_common_entrypoints.c"
        "--prefix"
        "vk_common"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_cmd_queue
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_cmd_queue_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_queue.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_queue.h"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_queue.c"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_queue.h"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_cmd_enqueue_entrypoints
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_entrypoints_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_enqueue_entrypoints.h"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_enqueue_entrypoints.c"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--proto"
        "--weak"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_enqueue_entrypoints.h"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_enqueue_entrypoints.c"
        "--prefix"
        "vk_cmd_enqueue"
        "--prefix"
        "vk_cmd_enqueue_unless_primary"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_dispatch_trampolines
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_dispatch_trampolines_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_dispatch_trampolines.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_dispatch_trampolines.h"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_dispatch_trampolines.c"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_dispatch_trampolines.h"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_physical_device_features
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_physical_device_features_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_features.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_features.h"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_features.c"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_features.h"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_physical_device_properties
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_physical_device_properties_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_properties.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_properties.h"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_properties.c"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_properties.h"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_physical_device_spirv_caps
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_physical_device_spirv_caps_gen.py"
    INPUTS
        ${_vk_py_inputs}
        "${_mesa_src_dir}/src/src/vulkan/util/vk_physical_device_features_gen.py"
    OUTPUTS "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_spirv_caps.c"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_spirv_caps.c"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_synchronization_helpers
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_synchronization_helpers_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS "${_mesa_gen_dir}/src/vulkan/runtime/vk_synchronization_helpers.c"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_synchronization_helpers.c"
        "--beta"
        "false"
)

_mesa_action(mesa_vk_format_info
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/runtime/vk_format_info_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_format_info.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_format_info.h"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_format_info.c"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_format_info.h"
)

# GLSL -> SPIR-V header codegen using glslang-standalone
set(_bvh_sources
    "src/src/vulkan/runtime/bvh/hploc_internal.comp"
    "src/src/vulkan/runtime/bvh/id_prefix_sum.comp"
    "src/src/vulkan/runtime/bvh/lbvh_generate_ir.comp"
    "src/src/vulkan/runtime/bvh/lbvh_main.comp"
    "src/src/vulkan/runtime/bvh/leaf.comp"
    "src/src/vulkan/runtime/bvh/pair_triangles.comp"
    "src/src/vulkan/runtime/bvh/ploc_internal.comp"
)
set(_bvh_inputs
    "${_mesa_src_dir}/src/src/compiler/spirv/spirv_internal_exts.h"
    "${_mesa_src_dir}/src/src/vulkan/runtime/bvh/leaf.h"
    "${_mesa_src_dir}/src/src/vulkan/runtime/bvh/pair_triangles.h"
    "${_mesa_src_dir}/src/src/vulkan/runtime/bvh/vk_bvh_defines.h"
    "${_mesa_src_dir}/src/src/vulkan/runtime/bvh/vk_bvh_helpers.h"
    "${_mesa_src_dir}/src/src/vulkan/runtime/bvh/vk_debug.h"
)
set(_bvh_preamble_args
    --quiet
    "-P#extension GL_GOOGLE_include_directive : require"
    "-P#extension GL_EXT_control_flow_attributes : require"
    "-P#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require"
    "-P#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require"
    "-P#extension GL_EXT_shader_explicit_arithmetic_types_int32 : require"
    "-P#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require"
    "-P#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require"
    "-P#extension GL_EXT_scalar_block_layout : require"
    "-P#extension GL_EXT_buffer_reference : require"
    "-P#extension GL_EXT_buffer_reference2 : require"
    "-P#extension GL_KHR_memory_scope_semantics : require"
    "-P#extension GL_KHR_shader_subgroup_arithmetic : require"
    "-P#extension GL_KHR_shader_subgroup_basic : require"
    "-P#extension GL_KHR_shader_subgroup_shuffle : require"
    "-P#extension GL_KHR_shader_subgroup_ballot : require"
    "-P#extension GL_KHR_shader_subgroup_clustered : require"
    "-P#extension GL_KHR_shader_subgroup_vote : require"
    "-P#extension GL_EXT_shader_atomic_int64 : require"
    "-P#extension GL_EXT_spirv_intrinsics : require"
    "-P#extension GL_EXT_shared_memory_block : require"
)
foreach(_shader IN LISTS _bvh_sources)
    get_filename_component(_name_part "${_shader}" NAME_WE)
    set(_out "${_mesa_gen_dir}/src/vulkan/runtime/bvh/${_name_part}.spv.h")
    add_custom_command(
        OUTPUT "${_out}"
        COMMAND $<TARGET_FILE:glslang-standalone>
            -V
            "-I${_mesa_src_dir}/src/src/vulkan/runtime/bvh"
            "-I${_mesa_src_dir}/src/src/compiler/spirv"
            --target-env spirv1.5
            -x
            -o "${_out}"
            "${_mesa_src_dir}/${_shader}"
            ${_bvh_preamble_args}
        DEPENDS
            glslang-standalone
            "${_mesa_src_dir}/${_shader}"
            ${_bvh_inputs}
        VERBATIM
    )
    list(APPEND _mesa_all_codegen_outputs "${_out}")
endforeach()

set(_morton_sort_src "${_mesa_src_dir}/src/src/vulkan/runtime/bvh/morton_sort.comp")
set(_morton_sort_32_out "${_mesa_gen_dir}/src/vulkan/runtime/bvh/morton_sort_32.spv.h")
add_custom_command(
    OUTPUT "${_morton_sort_32_out}"
    COMMAND $<TARGET_FILE:glslang-standalone>
        -V
        "-I${_mesa_src_dir}/src/src/vulkan/runtime/bvh"
        "-I${_mesa_src_dir}/src/src/compiler/spirv"
        --target-env spirv1.5
        -x
        -o "${_morton_sort_32_out}"
        "${_morton_sort_src}"
        ${_bvh_preamble_args}
    DEPENDS
        glslang-standalone
        "${_morton_sort_src}"
        ${_bvh_inputs}
    VERBATIM
)
list(APPEND _mesa_all_codegen_outputs "${_morton_sort_32_out}")

set(_morton_sort_64_out "${_mesa_gen_dir}/src/vulkan/runtime/bvh/morton_sort_64.spv.h")
add_custom_command(
    OUTPUT "${_morton_sort_64_out}"
    COMMAND $<TARGET_FILE:glslang-standalone>
        -V
        "-I${_mesa_src_dir}/src/src/vulkan/runtime/bvh"
        "-I${_mesa_src_dir}/src/src/compiler/spirv"
        --target-env spirv1.5
        -x
        -o "${_morton_sort_64_out}"
        "${_morton_sort_src}"
        -DMORTON_SORT_KEY64=1
        ${_bvh_preamble_args}
    DEPENDS
        glslang-standalone
        "${_morton_sort_src}"
        ${_bvh_inputs}
    VERBATIM
)
list(APPEND _mesa_all_codegen_outputs "${_morton_sort_64_out}")

set(_astc_out "${_mesa_gen_dir}/src/vulkan/runtime/astc_spv.h")
add_custom_command(
    OUTPUT "${_astc_out}"
    COMMAND $<TARGET_FILE:glslang-standalone>
        -V
        --target-env spirv1.2
        -S comp
        -x
        -o "${_astc_out}"
        "${_mesa_src_dir}/src/src/compiler/glsl/astc_decoder.glsl"
        --quiet
    DEPENDS
        glslang-standalone
        "${_mesa_src_dir}/src/src/compiler/glsl/astc_decoder.glsl"
    VERBATIM
)
list(APPEND _mesa_all_codegen_outputs "${_astc_out}")

_mesa_action(mesa_wsi_entrypoints
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_entrypoints_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/vulkan/wsi/wsi_common_entrypoints.h"
        "${_mesa_gen_dir}/src/vulkan/wsi/wsi_common_entrypoints.c"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--proto"
        "--weak"
        "--out-h"
        "${_mesa_gen_dir}/src/vulkan/wsi/wsi_common_entrypoints.h"
        "--out-c"
        "${_mesa_gen_dir}/src/vulkan/wsi/wsi_common_entrypoints.c"
        "--prefix"
        "wsi"
        "--beta"
        "false"
)

_mesa_action(mesa_tr_util
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/gallium/auxiliary/driver_trace/enums2names.py"
    INPUTS
        "${_mesa_src_dir}/src/src/gallium/include/pipe/p_defines.h"
        "${_mesa_src_dir}/src/src/gallium/include/pipe/p_video_enums.h"
        "${_mesa_src_dir}/src/src/util/blend.h"
    OUTPUTS
        "${_mesa_gen_dir}/src/gallium/auxiliary/tr_util.c"
        "${_mesa_gen_dir}/src/gallium/auxiliary/tr_util.h"
    ARGS
        "${_mesa_src_dir}/src/src/gallium/include/pipe/p_defines.h"
        "${_mesa_src_dir}/src/src/gallium/include/pipe/p_video_enums.h"
        "${_mesa_src_dir}/src/src/util/blend.h"
        "-C"
        "${_mesa_gen_dir}/src/gallium/auxiliary/tr_util.c"
        "-H"
        "${_mesa_gen_dir}/src/gallium/auxiliary/tr_util.h"
)

_mesa_action(mesa_u_tracepoints_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/gallium/auxiliary/util/u_tracepoints.py"
    INPUTS "${_mesa_src_dir}/src/src/util/perf/u_trace.py"
    OUTPUTS "${_mesa_gen_dir}/src/gallium/auxiliary/u_tracepoints.c"
    ARGS
        "-p"
        "${_mesa_src_dir}/src/src/util/perf"
        "-C"
        "${_mesa_gen_dir}/src/gallium/auxiliary/u_tracepoints.c"
)

_mesa_action(mesa_u_tracepoints_h
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/gallium/auxiliary/util/u_tracepoints.py"
    INPUTS "${_mesa_src_dir}/src/src/util/perf/u_trace.py"
    OUTPUTS "${_mesa_gen_dir}/src/gallium/auxiliary/u_tracepoints.h"
    ARGS
        "-p"
        "${_mesa_src_dir}/src/src/util/perf"
        "-H"
        "${_mesa_gen_dir}/src/gallium/auxiliary/u_tracepoints.h"
)

_mesa_action(mesa_u_indices_gen_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/gallium/auxiliary/indices/u_indices_gen.py"
    OUTPUTS "${_mesa_gen_dir}/src/gallium/auxiliary/u_indices_gen.c"
    ARGS "${_mesa_gen_dir}/src/gallium/auxiliary/u_indices_gen.c"
)

_mesa_action(mesa_u_unfilled_gen_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/gallium/auxiliary/indices/u_unfilled_gen.py"
    OUTPUTS "${_mesa_gen_dir}/src/gallium/auxiliary/u_unfilled_gen.c"
    ARGS "${_mesa_gen_dir}/src/gallium/auxiliary/u_unfilled_gen.c"
)

_mesa_action(mesa_draw_nir_lower_opcodes_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/gallium/auxiliary/draw/draw_nir_lower_opcodes.py"
    INPUTS
        ${_nir_py_inputs}
        "${_mesa_src_dir}/src/src/compiler/nir/nir_algebraic.py"
    OUTPUTS "${_mesa_gen_dir}/src/gallium/auxiliary/draw_nir_lower_opcodes.c"
    CAPTURE_STDOUT
    ARGS
        "-p"
        "${_mesa_src_dir}/src/src/compiler/nir"
)

_mesa_action(mesa_lp_bld_nir_no_integer_algebraic_c
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/gallium/auxiliary/gallivm/lp_bld_nir_no_integer_algebraic.py"
    INPUTS
        ${_nir_py_inputs}
        "${_mesa_src_dir}/src/src/compiler/nir/nir_algebraic.py"
    OUTPUTS "${_mesa_gen_dir}/src/gallium/auxiliary/lp_bld_nir_no_integer_algebraic.c"
    CAPTURE_STDOUT
    ARGS
        "-p"
        "${_mesa_src_dir}/src/src/compiler/nir"
)

_mesa_action(mesa_lvp_drirc
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/gallium/frontends/lavapipe/lvp_drirc_gen.py"
    INPUTS
        "${_mesa_src_dir}/src/src/gallium/frontends/lavapipe/00-lavapipe-defaults.conf"
        "${_mesa_src_dir}/src/src/util/drirc_gen.py"
    OUTPUTS
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_drirc.c"
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_drirc.h"
    ARGS
        "--import-path"
        "${_mesa_src_dir}/src/src/util"
        "--drirc-src"
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_drirc.c"
        "--drirc-hdr"
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_drirc.h"
        "--validate"
        "${_mesa_src_dir}/src/src/gallium/frontends/lavapipe/00-lavapipe-defaults.conf"
)

_mesa_action(mesa_lvp_entrypoints
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_entrypoints_gen.py"
    INPUTS ${_vk_py_inputs}
    OUTPUTS
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_entrypoints.h"
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_entrypoints.c"
    ARGS
        "--xml"
        "${_vk_xml}"
        "--proto"
        "--weak"
        "--out-h"
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_entrypoints.h"
        "--out-c"
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_entrypoints.c"
        "--prefix"
        "lvp"
        "--beta"
        "false"
)

_mesa_action(mesa_icd_json
    PYTHON_SCRIPT "${_mesa_src_dir}/src/src/vulkan/util/vk_icd_gen.py"
    INPUTS "${_vk_xml}"
    OUTPUTS "${_mesa_icd_json}"
    ARGS
        "--api-version"
        "1.4"
        "--xml"
        "${_vk_xml}"
        "--sizeof-pointer"
        "8"
        "--icd-lib-path"
        "."
        "--icd-filename"
        "./libvulkan_lvp.so"
        "--out"
        "${_mesa_icd_json}"
)

add_custom_target(mesa_all_codegen DEPENDS ${_mesa_all_codegen_outputs})

# ---------------------------------------------------------------------------
# Compilation Config & Object Libraries
# ---------------------------------------------------------------------------

set(_mesa_include_dirs
    "${_mesa_gen_dir}/src"
    "${_mesa_gen_dir}/src/compiler"
    "${_mesa_gen_dir}/src/compiler/nir"
    "${_mesa_gen_dir}/src/compiler/spirv"
    "${_mesa_gen_dir}/src/gallium/auxiliary"
    "${_mesa_gen_dir}/src/gallium/frontends/lavapipe"
    "${_mesa_gen_dir}/src/util"
    "${_mesa_gen_dir}/src/util/format"
    "${_mesa_gen_dir}/src/vulkan/runtime"
    "${_mesa_gen_dir}/src/vulkan/runtime/bvh"
    "${_mesa_gen_dir}/src/vulkan/util"
    "${_mesa_gen_dir}/src/vulkan/wsi"
    "${_mesa_src_dir}/src/include"
    "${_mesa_src_dir}/src/src"
    "${_mesa_src_dir}/src/src/c11/impl"
    "${_mesa_src_dir}/src/src/compiler"
    "${_mesa_src_dir}/src/src/compiler/nir"
    "${_mesa_src_dir}/src/src/compiler/spirv"
    "${_mesa_src_dir}/src/src/gallium/auxiliary"
    "${_mesa_src_dir}/src/src/gallium/auxiliary/pipe-loader"
    "${_mesa_src_dir}/src/src/gallium/auxiliary/util"
    "${_mesa_src_dir}/src/src/gallium/drivers"
    "${_mesa_src_dir}/src/src/gallium/drivers/llvmpipe"
    "${_mesa_src_dir}/src/src/gallium/frontends/lavapipe"
    "${_mesa_src_dir}/src/src/gallium/include"
    "${_mesa_src_dir}/src/src/gallium/targets/lavapipe"
    "${_mesa_src_dir}/src/src/gallium/winsys"
    "${_mesa_src_dir}/src/src/gallium/winsys/sw/null"
    "${_mesa_src_dir}/src/src/gallium/winsys/sw/wrapper"
    "${_mesa_src_dir}/src/src/util"
    "${_mesa_src_dir}/src/src/util/blake3"
    "${_mesa_src_dir}/src/src/util/format"
    "${_mesa_src_dir}/src/src/vulkan/runtime"
    "${_mesa_src_dir}/src/src/vulkan/runtime/bvh"
    "${_mesa_src_dir}/src/src/vulkan/util"
    "${_mesa_src_dir}/src/src/vulkan/wsi"
    "${MESA_ZSTD_DIR}/include"
)

function(_mesa_source_set target_name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG
        ""
        ""
        "SOURCES;DEFINES;CFLAGS;DEPS"
    )
    set(_resolved_sources "")
    foreach(_src IN LISTS ARG_SOURCES)
        if(IS_ABSOLUTE "${_src}")
            list(APPEND _resolved_sources "${_src}")
        else()
            list(APPEND _resolved_sources "${_mesa_src_dir}/${_src}")
        endif()
    endforeach()

    add_library(${target_name} OBJECT ${_resolved_sources})
    set_target_properties(${target_name} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
    target_include_directories(${target_name} SYSTEM PRIVATE
        "${MESA_LLVM_DEV_DIR}/include"
    )
    target_include_directories(${target_name} PRIVATE
        ${_mesa_include_dirs}
    )
    target_compile_definitions(${target_name} PRIVATE
        ${mesa_common_defines}
        ${ARG_DEFINES}
    )
    target_compile_options(${target_name} PRIVATE
        "$<$<COMPILE_LANGUAGE:C,CXX>:${mesa_common_cflags}>"
        "$<$<COMPILE_LANGUAGE:C>:${mesa_common_cflags_c}>"
        "$<$<COMPILE_LANGUAGE:CXX>:${mesa_common_cflags_cc}>"
    )
    if(ARG_CFLAGS)
        target_compile_options(${target_name} PRIVATE
            "$<$<COMPILE_LANGUAGE:C,CXX>:${ARG_CFLAGS}>"
        )
    endif()
    if(DAWN_EMIT_COVERAGE AND (COMPILER_IS_CLANG OR COMPILER_IS_CLANG_CL))
        target_compile_options(${target_name} PRIVATE
            "$<$<COMPILE_LANGUAGE:C,CXX>:-fprofile-instr-generate;-fcoverage-mapping>"
        )
    endif()

    add_dependencies(${target_name} mesa_all_codegen)
    if(ARG_DEPS)
        add_dependencies(${target_name} ${ARG_DEPS})
    endif()
endfunction()

_mesa_source_set(mesa_util_c11
    SOURCES ${mesa_util_c11_sources}
)

_mesa_source_set(mesa_blake3
    SOURCES ${mesa_blake3_sources}
)

_mesa_source_set(mesa_util_clflush
    SOURCES ${mesa_util_clflush_sources}
)

_mesa_source_set(mesa_util_clflushopt
    SOURCES ${mesa_util_clflushopt_sources}
    CFLAGS "-mclflushopt"
)

_mesa_source_set(mesa_util_simd
    SOURCES ${mesa_util_simd_sources}
    CFLAGS "-msse4.1"
)

_mesa_source_set(mesa_xmlconfig
    SOURCES ${mesa_xmlconfig_sources}
    DEFINES "WITH_XMLCONFIG=0"
)

_mesa_source_set(mesa_util
    SOURCES
        ${mesa_util_static_sources}
        "${_mesa_gen_dir}/src/util/format/u_format_table.c"
        "${_mesa_gen_dir}/src/util/format_srgb.c"
    DEPS
        mesa_blake3
        mesa_util_c11
        mesa_util_clflush
        mesa_util_clflushopt
        mesa_util_simd
        mesa_xmlconfig
)

_mesa_source_set(mesa_compiler
    SOURCES
        ${mesa_compiler_static_sources}
        "${_mesa_gen_dir}/src/compiler/builtin_types.c"
    DEPS
        mesa_util
)

_mesa_source_set(mesa_nir
    SOURCES
        ${mesa_nir_static_sources}
        "${_mesa_gen_dir}/src/compiler/nir/nir_constant_expressions.c"
        "${_mesa_gen_dir}/src/compiler/nir/nir_intrinsics.c"
        "${_mesa_gen_dir}/src/compiler/nir/nir_opcodes.c"
        "${_mesa_gen_dir}/src/compiler/nir/nir_opt_algebraic.c"
    DEPS
        mesa_compiler
        mesa_util
)

_mesa_source_set(mesa_vtn
    SOURCES
        ${mesa_vtn_static_sources}
        "${_mesa_gen_dir}/src/compiler/spirv/spirv_info.c"
        "${_mesa_gen_dir}/src/compiler/spirv/vtn_gather_types.c"
    DEPS
        mesa_nir
        mesa_util
)

_mesa_source_set(mesa_vulkan_util
    SOURCES
        ${mesa_vulkan_util_static_sources}
        "${_mesa_gen_dir}/src/vulkan/util/vk_dispatch_table.c"
        "${_mesa_gen_dir}/src/vulkan/util/vk_enum_to_str.c"
        "${_mesa_gen_dir}/src/vulkan/util/vk_extensions.c"
    DEPS
        mesa_compiler
        mesa_util
)

_mesa_source_set(mesa_vulkan_runtime
    SOURCES
        ${mesa_vulkan_runtime_static_sources}
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_enqueue_entrypoints.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_cmd_queue.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_common_entrypoints.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_dispatch_trampolines.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_format_info.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_features.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_properties.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_physical_device_spirv_caps.c"
        "${_mesa_gen_dir}/src/vulkan/runtime/vk_synchronization_helpers.c"
        "${_mesa_gen_dir}/src/vulkan/wsi/wsi_common_entrypoints.c"
    DEFINES
        "MESA_VK_LOG=0"
        "VK_LITE_RUNTIME_INSTANCE=0"
    DEPS
        mesa_nir
        mesa_util
        mesa_vtn
        mesa_vulkan_util
)

_mesa_source_set(mesa_gallium_aux
    SOURCES
        ${mesa_gallium_aux_static_sources}
        "${_mesa_gen_dir}/src/gallium/auxiliary/draw_nir_lower_opcodes.c"
        "${_mesa_gen_dir}/src/gallium/auxiliary/lp_bld_nir_no_integer_algebraic.c"
        "${_mesa_gen_dir}/src/gallium/auxiliary/tr_util.c"
        "${_mesa_gen_dir}/src/gallium/auxiliary/u_indices_gen.c"
        "${_mesa_gen_dir}/src/gallium/auxiliary/u_tracepoints.c"
        "${_mesa_gen_dir}/src/gallium/auxiliary/u_unfilled_gen.c"
    DEPS
        mesa_nir
        mesa_util
)

_mesa_source_set(mesa_llvmpipe
    SOURCES ${mesa_llvmpipe_sources}
    DEPS
        mesa_gallium_aux
        mesa_nir
        mesa_util
)

_mesa_source_set(mesa_lavapipe
    SOURCES
        ${mesa_lavapipe_static_sources}
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_drirc.c"
        "${_mesa_gen_dir}/src/gallium/frontends/lavapipe/lvp_entrypoints.c"
        "mesa_llvm_sysroot_compat.cc"
    DEFINES
        "GALLIUM_LLVMPIPE"
        # lvp_formats.c references DRM_FORMAT_MOD_LINEAR under #if DETECT_OS_LINUX
        # even when HAVE_LIBDRM is not defined (which skips including
        # drm-uapi/drm_fourcc.h in lvp_private.h).
        "DRM_FORMAT_MOD_LINEAR=0"
    DEPS
        mesa_gallium_aux
        mesa_llvmpipe
        mesa_nir
        mesa_util
        mesa_vtn
        mesa_vulkan_runtime
        mesa_vulkan_util
)

add_library(vulkan_lvp SHARED
    $<TARGET_OBJECTS:mesa_util_c11>
    $<TARGET_OBJECTS:mesa_blake3>
    $<TARGET_OBJECTS:mesa_util_clflush>
    $<TARGET_OBJECTS:mesa_util_clflushopt>
    $<TARGET_OBJECTS:mesa_util_simd>
    $<TARGET_OBJECTS:mesa_xmlconfig>
    $<TARGET_OBJECTS:mesa_util>
    $<TARGET_OBJECTS:mesa_compiler>
    $<TARGET_OBJECTS:mesa_nir>
    $<TARGET_OBJECTS:mesa_vtn>
    $<TARGET_OBJECTS:mesa_vulkan_util>
    $<TARGET_OBJECTS:mesa_vulkan_runtime>
    $<TARGET_OBJECTS:mesa_gallium_aux>
    $<TARGET_OBJECTS:mesa_llvmpipe>
    $<TARGET_OBJECTS:mesa_lavapipe>
)

set_target_properties(vulkan_lvp PROPERTIES
    OUTPUT_NAME "vulkan_lvp"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "\$ORIGIN"
)

target_link_libraries(vulkan_lvp PRIVATE
    "${MESA_ZSTD_DIR}/lib/libzstd.so"
    -Wl,--start-group
    ${mesa_llvm_libs}
    -Wl,--end-group
    dl
    m
    pthread
    rt
    z
)

get_filename_component(_compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
find_program(_mesa_lld NAMES ld.lld lld HINTS "${_compiler_dir}")
if(_mesa_lld)
    target_link_options(vulkan_lvp PRIVATE "-fuse-ld=lld")
endif()

target_link_options(vulkan_lvp PRIVATE
    "-Wl,--no-undefined"
    "-Wl,-Bsymbolic"
    "-Wl,-z,nodelete"
    "-Wl,--gc-sections"
    "-Wl,--exclude-libs,ALL"
    "-Wl,-rpath,\$ORIGIN"
)

if(DAWN_EMIT_COVERAGE AND (COMPILER_IS_CLANG OR COMPILER_IS_CLANG_CL))
    target_link_options(vulkan_lvp PRIVATE
        "-fprofile-instr-generate"
        "-fcoverage-mapping"
    )
endif()

add_custom_target(mesa ALL
    DEPENDS
        vulkan_lvp
        mesa_copy_zstd
        "${_mesa_icd_json}"
)
