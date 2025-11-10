#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform ub_block_1_ubo {
  uvec4 inner[400];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  int inner;
} v_1;
int tint_f16_to_i32(float16_t value) {
  return int(clamp(value, -65504.0hf, 65504.0hf));
}
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  f16vec2 v_4 = tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]);
  f16vec2 v_5 = tint_bitcast_to_f16(v.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) & 15u) >> 2u)]);
  return f16mat4x2(v_3, v_4, v_5, tint_bitcast_to_f16(v.inner[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) & 15u) >> 2u)]));
}
f16mat4x2[2] v_6(uint start_byte_offset) {
  f16mat4x2 a[2] = f16mat4x2[2](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 2u)) {
        break;
      }
      a[v_8] = v_2((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  return a;
}
vec3[2] v_9(uint start_byte_offset) {
  vec3 a[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 2u)) {
        break;
      }
      a[v_11] = uintBitsToFloat(v.inner[((start_byte_offset + (v_11 * 16u)) / 16u)].xyz);
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
  return a;
}
f16vec4 tint_bitcast_to_f16_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_12(uint start_byte_offset) {
  uvec4 v_13 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_14 = tint_bitcast_to_f16_1(mix(v_13.xy, v_13.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_15 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_16 = tint_bitcast_to_f16_1(mix(v_15.xy, v_15.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_17 = v.inner[((16u + start_byte_offset) / 16u)];
  f16vec4 v_18 = tint_bitcast_to_f16_1(mix(v_17.xy, v_17.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_19 = v.inner[((24u + start_byte_offset) / 16u)];
  return f16mat4(v_14, v_16, v_18, tint_bitcast_to_f16_1(mix(v_19.xy, v_19.zw, bvec2(((((24u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
f16mat4x3 v_20(uint start_byte_offset) {
  uvec4 v_21 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_22 = tint_bitcast_to_f16_1(mix(v_21.xy, v_21.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_23 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec3 v_24 = tint_bitcast_to_f16_1(mix(v_23.xy, v_23.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_25 = v.inner[((16u + start_byte_offset) / 16u)];
  f16vec3 v_26 = tint_bitcast_to_f16_1(mix(v_25.xy, v_25.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_27 = v.inner[((24u + start_byte_offset) / 16u)];
  return f16mat4x3(v_22, v_24, v_26, tint_bitcast_to_f16_1(mix(v_27.xy, v_27.zw, bvec2(((((24u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz);
}
f16mat3x4 v_28(uint start_byte_offset) {
  uvec4 v_29 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_30 = tint_bitcast_to_f16_1(mix(v_29.xy, v_29.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_31 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_32 = tint_bitcast_to_f16_1(mix(v_31.xy, v_31.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_33 = v.inner[((16u + start_byte_offset) / 16u)];
  return f16mat3x4(v_30, v_32, tint_bitcast_to_f16_1(mix(v_33.xy, v_33.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
f16mat3 v_34(uint start_byte_offset) {
  uvec4 v_35 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_36 = tint_bitcast_to_f16_1(mix(v_35.xy, v_35.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_37 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec3 v_38 = tint_bitcast_to_f16_1(mix(v_37.xy, v_37.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_39 = v.inner[((16u + start_byte_offset) / 16u)];
  return f16mat3(v_36, v_38, tint_bitcast_to_f16_1(mix(v_39.xy, v_39.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz);
}
f16mat3x2 v_40(uint start_byte_offset) {
  f16vec2 v_41 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  f16vec2 v_42 = tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]);
  return f16mat3x2(v_41, v_42, tint_bitcast_to_f16(v.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) & 15u) >> 2u)]));
}
f16mat2x4 v_43(uint start_byte_offset) {
  uvec4 v_44 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_45 = tint_bitcast_to_f16_1(mix(v_44.xy, v_44.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_46 = v.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x4(v_45, tint_bitcast_to_f16_1(mix(v_46.xy, v_46.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
f16mat2x3 v_47(uint start_byte_offset) {
  uvec4 v_48 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_49 = tint_bitcast_to_f16_1(mix(v_48.xy, v_48.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_50 = v.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x3(v_49, tint_bitcast_to_f16_1(mix(v_50.xy, v_50.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz);
}
f16mat2 v_51(uint start_byte_offset) {
  f16vec2 v_52 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  return f16mat2(v_52, tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]));
}
mat4 v_53(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_54(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_55(uint start_byte_offset) {
  uvec4 v_56 = v.inner[(start_byte_offset / 16u)];
  vec2 v_57 = uintBitsToFloat(mix(v_56.xy, v_56.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_58 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_59 = uintBitsToFloat(mix(v_58.xy, v_58.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_60 = v.inner[((16u + start_byte_offset) / 16u)];
  vec2 v_61 = uintBitsToFloat(mix(v_60.xy, v_60.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_62 = v.inner[((24u + start_byte_offset) / 16u)];
  return mat4x2(v_57, v_59, v_61, uintBitsToFloat(mix(v_62.xy, v_62.zw, bvec2(((((24u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
mat3x4 v_63(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_64(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_65(uint start_byte_offset) {
  uvec4 v_66 = v.inner[(start_byte_offset / 16u)];
  vec2 v_67 = uintBitsToFloat(mix(v_66.xy, v_66.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_68 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_69 = uintBitsToFloat(mix(v_68.xy, v_68.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_70 = v.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_67, v_69, uintBitsToFloat(mix(v_70.xy, v_70.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
mat2x4 v_71(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_72(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_73(uint start_byte_offset) {
  uvec4 v_74 = v.inner[(start_byte_offset / 16u)];
  vec2 v_75 = uintBitsToFloat(mix(v_74.xy, v_74.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_76 = v.inner[((8u + start_byte_offset) / 16u)];
  return mat2(v_75, uintBitsToFloat(mix(v_76.xy, v_76.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
void main_inner(uint idx) {
  uvec4 v_77 = v.inner[((800u * idx) / 16u)];
  float scalar_f32 = uintBitsToFloat(v_77[(((800u * idx) & 15u) >> 2u)]);
  uvec4 v_78 = v.inner[((4u + (800u * idx)) / 16u)];
  int scalar_i32 = int(v_78[(((4u + (800u * idx)) & 15u) >> 2u)]);
  uvec4 v_79 = v.inner[((8u + (800u * idx)) / 16u)];
  uint scalar_u32 = v_79[(((8u + (800u * idx)) & 15u) >> 2u)];
  uvec4 v_80 = v.inner[((12u + (800u * idx)) / 16u)];
  float16_t scalar_f16 = tint_bitcast_to_f16(v_80[(((12u + (800u * idx)) & 15u) >> 2u)])[mix(1u, 0u, (((12u + (800u * idx)) % 4u) == 0u))];
  uvec4 v_81 = v.inner[((16u + (800u * idx)) / 16u)];
  vec2 vec2_f32 = uintBitsToFloat(mix(v_81.xy, v_81.zw, bvec2(((((16u + (800u * idx)) & 15u) >> 2u) == 2u))));
  uvec4 v_82 = v.inner[((24u + (800u * idx)) / 16u)];
  ivec2 vec2_i32 = ivec2(mix(v_82.xy, v_82.zw, bvec2(((((24u + (800u * idx)) & 15u) >> 2u) == 2u))));
  uvec4 v_83 = v.inner[((32u + (800u * idx)) / 16u)];
  uvec2 vec2_u32 = mix(v_83.xy, v_83.zw, bvec2(((((32u + (800u * idx)) & 15u) >> 2u) == 2u)));
  f16vec2 vec2_f16 = tint_bitcast_to_f16(v.inner[((40u + (800u * idx)) / 16u)][(((40u + (800u * idx)) & 15u) >> 2u)]);
  vec3 vec3_f32 = uintBitsToFloat(v.inner[((48u + (800u * idx)) / 16u)].xyz);
  ivec3 vec3_i32 = ivec3(v.inner[((64u + (800u * idx)) / 16u)].xyz);
  uvec3 vec3_u32 = v.inner[((80u + (800u * idx)) / 16u)].xyz;
  uvec4 v_84 = v.inner[((96u + (800u * idx)) / 16u)];
  f16vec3 vec3_f16 = tint_bitcast_to_f16_1(mix(v_84.xy, v_84.zw, bvec2(((((96u + (800u * idx)) & 15u) >> 2u) == 2u)))).xyz;
  vec4 vec4_f32 = uintBitsToFloat(v.inner[((112u + (800u * idx)) / 16u)]);
  ivec4 vec4_i32 = ivec4(v.inner[((128u + (800u * idx)) / 16u)]);
  uvec4 vec4_u32 = v.inner[((144u + (800u * idx)) / 16u)];
  uvec4 v_85 = v.inner[((160u + (800u * idx)) / 16u)];
  f16vec4 vec4_f16 = tint_bitcast_to_f16_1(mix(v_85.xy, v_85.zw, bvec2(((((160u + (800u * idx)) & 15u) >> 2u) == 2u))));
  mat2 mat2x2_f32 = v_73((168u + (800u * idx)));
  mat2x3 mat2x3_f32 = v_72((192u + (800u * idx)));
  mat2x4 mat2x4_f32 = v_71((224u + (800u * idx)));
  mat3x2 mat3x2_f32 = v_65((256u + (800u * idx)));
  mat3 mat3x3_f32 = v_64((288u + (800u * idx)));
  mat3x4 mat3x4_f32 = v_63((336u + (800u * idx)));
  mat4x2 mat4x2_f32 = v_55((384u + (800u * idx)));
  mat4x3 mat4x3_f32 = v_54((416u + (800u * idx)));
  mat4 mat4x4_f32 = v_53((480u + (800u * idx)));
  f16mat2 mat2x2_f16 = v_51((544u + (800u * idx)));
  f16mat2x3 mat2x3_f16 = v_47((552u + (800u * idx)));
  f16mat2x4 mat2x4_f16 = v_43((568u + (800u * idx)));
  f16mat3x2 mat3x2_f16 = v_40((584u + (800u * idx)));
  f16mat3 mat3x3_f16 = v_34((600u + (800u * idx)));
  f16mat3x4 mat3x4_f16 = v_28((624u + (800u * idx)));
  f16mat4x2 mat4x2_f16 = v_2((648u + (800u * idx)));
  f16mat4x3 mat4x3_f16 = v_20((664u + (800u * idx)));
  f16mat4 mat4x4_f16 = v_12((696u + (800u * idx)));
  vec3 arr2_vec3_f32[2] = v_9((736u + (800u * idx)));
  f16mat4x2 arr2_mat4x2_f16[2] = v_6((768u + (800u * idx)));
  uint v_86 = uint(tint_f32_to_i32(scalar_f32));
  int v_87 = int((v_86 + uint(scalar_i32)));
  int v_88 = int(scalar_u32);
  uint v_89 = uint(v_87);
  int v_90 = int((v_89 + uint(v_88)));
  int v_91 = tint_f16_to_i32(scalar_f16);
  uint v_92 = uint(v_90);
  int v_93 = int((v_92 + uint(v_91)));
  int v_94 = tint_f32_to_i32(vec2_f32.x);
  uint v_95 = uint(v_93);
  uint v_96 = uint(int((v_95 + uint(v_94))));
  int v_97 = int((v_96 + uint(vec2_i32.x)));
  int v_98 = int(vec2_u32.x);
  uint v_99 = uint(v_97);
  int v_100 = int((v_99 + uint(v_98)));
  int v_101 = tint_f16_to_i32(vec2_f16.x);
  uint v_102 = uint(v_100);
  int v_103 = int((v_102 + uint(v_101)));
  int v_104 = tint_f32_to_i32(vec3_f32.y);
  uint v_105 = uint(v_103);
  uint v_106 = uint(int((v_105 + uint(v_104))));
  int v_107 = int((v_106 + uint(vec3_i32.y)));
  int v_108 = int(vec3_u32.y);
  uint v_109 = uint(v_107);
  int v_110 = int((v_109 + uint(v_108)));
  int v_111 = tint_f16_to_i32(vec3_f16.y);
  uint v_112 = uint(v_110);
  int v_113 = int((v_112 + uint(v_111)));
  int v_114 = tint_f32_to_i32(vec4_f32.z);
  uint v_115 = uint(v_113);
  uint v_116 = uint(int((v_115 + uint(v_114))));
  int v_117 = int((v_116 + uint(vec4_i32.z)));
  int v_118 = int(vec4_u32.z);
  uint v_119 = uint(v_117);
  int v_120 = int((v_119 + uint(v_118)));
  int v_121 = tint_f16_to_i32(vec4_f16.z);
  uint v_122 = uint(v_120);
  int v_123 = int((v_122 + uint(v_121)));
  int v_124 = tint_f32_to_i32(mat2x2_f32[0u].x);
  uint v_125 = uint(v_123);
  int v_126 = int((v_125 + uint(v_124)));
  int v_127 = tint_f32_to_i32(mat2x3_f32[0u].x);
  uint v_128 = uint(v_126);
  int v_129 = int((v_128 + uint(v_127)));
  int v_130 = tint_f32_to_i32(mat2x4_f32[0u].x);
  uint v_131 = uint(v_129);
  int v_132 = int((v_131 + uint(v_130)));
  int v_133 = tint_f32_to_i32(mat3x2_f32[0u].x);
  uint v_134 = uint(v_132);
  int v_135 = int((v_134 + uint(v_133)));
  int v_136 = tint_f32_to_i32(mat3x3_f32[0u].x);
  uint v_137 = uint(v_135);
  int v_138 = int((v_137 + uint(v_136)));
  int v_139 = tint_f32_to_i32(mat3x4_f32[0u].x);
  uint v_140 = uint(v_138);
  int v_141 = int((v_140 + uint(v_139)));
  int v_142 = tint_f32_to_i32(mat4x2_f32[0u].x);
  uint v_143 = uint(v_141);
  int v_144 = int((v_143 + uint(v_142)));
  int v_145 = tint_f32_to_i32(mat4x3_f32[0u].x);
  uint v_146 = uint(v_144);
  int v_147 = int((v_146 + uint(v_145)));
  int v_148 = tint_f32_to_i32(mat4x4_f32[0u].x);
  uint v_149 = uint(v_147);
  int v_150 = int((v_149 + uint(v_148)));
  int v_151 = tint_f16_to_i32(mat2x2_f16[0u].x);
  uint v_152 = uint(v_150);
  int v_153 = int((v_152 + uint(v_151)));
  int v_154 = tint_f16_to_i32(mat2x3_f16[0u].x);
  uint v_155 = uint(v_153);
  int v_156 = int((v_155 + uint(v_154)));
  int v_157 = tint_f16_to_i32(mat2x4_f16[0u].x);
  uint v_158 = uint(v_156);
  int v_159 = int((v_158 + uint(v_157)));
  int v_160 = tint_f16_to_i32(mat3x2_f16[0u].x);
  uint v_161 = uint(v_159);
  int v_162 = int((v_161 + uint(v_160)));
  int v_163 = tint_f16_to_i32(mat3x3_f16[0u].x);
  uint v_164 = uint(v_162);
  int v_165 = int((v_164 + uint(v_163)));
  int v_166 = tint_f16_to_i32(mat3x4_f16[0u].x);
  uint v_167 = uint(v_165);
  int v_168 = int((v_167 + uint(v_166)));
  int v_169 = tint_f16_to_i32(mat4x2_f16[0u].x);
  uint v_170 = uint(v_168);
  int v_171 = int((v_170 + uint(v_169)));
  int v_172 = tint_f16_to_i32(mat4x3_f16[0u].x);
  uint v_173 = uint(v_171);
  int v_174 = int((v_173 + uint(v_172)));
  int v_175 = tint_f16_to_i32(mat4x4_f16[0u].x);
  uint v_176 = uint(v_174);
  int v_177 = int((v_176 + uint(v_175)));
  int v_178 = tint_f32_to_i32(arr2_vec3_f32[0u].x);
  uint v_179 = uint(v_177);
  int v_180 = int((v_179 + uint(v_178)));
  int v_181 = tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x);
  uint v_182 = uint(v_180);
  v_1.inner = int((v_182 + uint(v_181)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
