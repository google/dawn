#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};

layout(binding = 0, std140)
uniform ub_block_1_ubo {
  uvec4 inner[55];
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
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
Inner v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  int v_4 = int(v_3[((start_byte_offset & 15u) >> 2u)]);
  uint v_5 = (4u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  uint v_7 = (8u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  return Inner(v_4, uintBitsToFloat(v_6[((v_5 & 15u) >> 2u)]), tint_bitcast_to_16bit(v_8[((v_7 & 15u) >> 2u)])[mix(1u, 0u, ((v_7 % 4u) == 0u))]);
}
Inner[4] v_9(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(0, 0.0f, 0.0hf), Inner(0, 0.0f, 0.0hf), Inner(0, 0.0f, 0.0hf), Inner(0, 0.0f, 0.0hf));
  {
    uint idx = 0u;
    while(true) {
      uint v_10 = idx;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_2((start_byte_offset + (v_10 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
f16mat4x2 v_11(uint start_byte_offset) {
  f16vec2 v_12 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_13 = (4u + start_byte_offset);
  f16vec2 v_14 = tint_bitcast_to_16bit(v.inner[(v_13 / 16u)][((v_13 & 15u) >> 2u)]);
  uint v_15 = (8u + start_byte_offset);
  f16vec2 v_16 = tint_bitcast_to_16bit(v.inner[(v_15 / 16u)][((v_15 & 15u) >> 2u)]);
  uint v_17 = (12u + start_byte_offset);
  return f16mat4x2(v_12, v_14, v_16, tint_bitcast_to_16bit(v.inner[(v_17 / 16u)][((v_17 & 15u) >> 2u)]));
}
f16mat4x2[2] v_18(uint start_byte_offset) {
  f16mat4x2 a[2] = f16mat4x2[2](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint idx = 0u;
    while(true) {
      uint v_19 = idx;
      if ((v_19 >= 2u)) {
        break;
      }
      a[v_19] = v_11((start_byte_offset + (v_19 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
vec3[2] v_20(uint start_byte_offset) {
  vec3 a[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  {
    uint idx = 0u;
    while(true) {
      uint v_21 = idx;
      if ((v_21 >= 2u)) {
        break;
      }
      a[v_21] = uintBitsToFloat(v.inner[((start_byte_offset + (v_21 * 16u)) / 16u)].xyz);
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
f16vec4 tint_bitcast_to_16bit_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_22(uint start_byte_offset) {
  uvec4 v_23 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_24 = tint_bitcast_to_16bit_1(mix(v_23.xy, v_23.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_25 = (8u + start_byte_offset);
  uvec4 v_26 = v.inner[(v_25 / 16u)];
  f16vec4 v_27 = tint_bitcast_to_16bit_1(mix(v_26.xy, v_26.zw, bvec2((((v_25 & 15u) >> 2u) == 2u))));
  uint v_28 = (16u + start_byte_offset);
  uvec4 v_29 = v.inner[(v_28 / 16u)];
  f16vec4 v_30 = tint_bitcast_to_16bit_1(mix(v_29.xy, v_29.zw, bvec2((((v_28 & 15u) >> 2u) == 2u))));
  uint v_31 = (24u + start_byte_offset);
  uvec4 v_32 = v.inner[(v_31 / 16u)];
  return f16mat4(v_24, v_27, v_30, tint_bitcast_to_16bit_1(mix(v_32.xy, v_32.zw, bvec2((((v_31 & 15u) >> 2u) == 2u)))));
}
f16mat4x3 v_33(uint start_byte_offset) {
  uvec4 v_34 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_35 = tint_bitcast_to_16bit_1(mix(v_34.xy, v_34.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_36 = (8u + start_byte_offset);
  uvec4 v_37 = v.inner[(v_36 / 16u)];
  f16vec3 v_38 = tint_bitcast_to_16bit_1(mix(v_37.xy, v_37.zw, bvec2((((v_36 & 15u) >> 2u) == 2u)))).xyz;
  uint v_39 = (16u + start_byte_offset);
  uvec4 v_40 = v.inner[(v_39 / 16u)];
  f16vec3 v_41 = tint_bitcast_to_16bit_1(mix(v_40.xy, v_40.zw, bvec2((((v_39 & 15u) >> 2u) == 2u)))).xyz;
  uint v_42 = (24u + start_byte_offset);
  uvec4 v_43 = v.inner[(v_42 / 16u)];
  return f16mat4x3(v_35, v_38, v_41, tint_bitcast_to_16bit_1(mix(v_43.xy, v_43.zw, bvec2((((v_42 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat3x4 v_44(uint start_byte_offset) {
  uvec4 v_45 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_46 = tint_bitcast_to_16bit_1(mix(v_45.xy, v_45.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_47 = (8u + start_byte_offset);
  uvec4 v_48 = v.inner[(v_47 / 16u)];
  f16vec4 v_49 = tint_bitcast_to_16bit_1(mix(v_48.xy, v_48.zw, bvec2((((v_47 & 15u) >> 2u) == 2u))));
  uint v_50 = (16u + start_byte_offset);
  uvec4 v_51 = v.inner[(v_50 / 16u)];
  return f16mat3x4(v_46, v_49, tint_bitcast_to_16bit_1(mix(v_51.xy, v_51.zw, bvec2((((v_50 & 15u) >> 2u) == 2u)))));
}
f16mat3 v_52(uint start_byte_offset) {
  uvec4 v_53 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_54 = tint_bitcast_to_16bit_1(mix(v_53.xy, v_53.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_55 = (8u + start_byte_offset);
  uvec4 v_56 = v.inner[(v_55 / 16u)];
  f16vec3 v_57 = tint_bitcast_to_16bit_1(mix(v_56.xy, v_56.zw, bvec2((((v_55 & 15u) >> 2u) == 2u)))).xyz;
  uint v_58 = (16u + start_byte_offset);
  uvec4 v_59 = v.inner[(v_58 / 16u)];
  return f16mat3(v_54, v_57, tint_bitcast_to_16bit_1(mix(v_59.xy, v_59.zw, bvec2((((v_58 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat3x2 v_60(uint start_byte_offset) {
  f16vec2 v_61 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_62 = (4u + start_byte_offset);
  f16vec2 v_63 = tint_bitcast_to_16bit(v.inner[(v_62 / 16u)][((v_62 & 15u) >> 2u)]);
  uint v_64 = (8u + start_byte_offset);
  return f16mat3x2(v_61, v_63, tint_bitcast_to_16bit(v.inner[(v_64 / 16u)][((v_64 & 15u) >> 2u)]));
}
f16mat2x4 v_65(uint start_byte_offset) {
  uvec4 v_66 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_67 = tint_bitcast_to_16bit_1(mix(v_66.xy, v_66.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_68 = (8u + start_byte_offset);
  uvec4 v_69 = v.inner[(v_68 / 16u)];
  return f16mat2x4(v_67, tint_bitcast_to_16bit_1(mix(v_69.xy, v_69.zw, bvec2((((v_68 & 15u) >> 2u) == 2u)))));
}
f16mat2x3 v_70(uint start_byte_offset) {
  uvec4 v_71 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_72 = tint_bitcast_to_16bit_1(mix(v_71.xy, v_71.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_73 = (8u + start_byte_offset);
  uvec4 v_74 = v.inner[(v_73 / 16u)];
  return f16mat2x3(v_72, tint_bitcast_to_16bit_1(mix(v_74.xy, v_74.zw, bvec2((((v_73 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat2 v_75(uint start_byte_offset) {
  f16vec2 v_76 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_77 = (4u + start_byte_offset);
  return f16mat2(v_76, tint_bitcast_to_16bit(v.inner[(v_77 / 16u)][((v_77 & 15u) >> 2u)]));
}
mat4 v_78(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_79(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_80(uint start_byte_offset) {
  uvec4 v_81 = v.inner[(start_byte_offset / 16u)];
  vec2 v_82 = uintBitsToFloat(mix(v_81.xy, v_81.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_83 = (8u + start_byte_offset);
  uvec4 v_84 = v.inner[(v_83 / 16u)];
  vec2 v_85 = uintBitsToFloat(mix(v_84.xy, v_84.zw, bvec2((((v_83 & 15u) >> 2u) == 2u))));
  uint v_86 = (16u + start_byte_offset);
  uvec4 v_87 = v.inner[(v_86 / 16u)];
  vec2 v_88 = uintBitsToFloat(mix(v_87.xy, v_87.zw, bvec2((((v_86 & 15u) >> 2u) == 2u))));
  uint v_89 = (24u + start_byte_offset);
  uvec4 v_90 = v.inner[(v_89 / 16u)];
  return mat4x2(v_82, v_85, v_88, uintBitsToFloat(mix(v_90.xy, v_90.zw, bvec2((((v_89 & 15u) >> 2u) == 2u)))));
}
mat3x4 v_91(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_92(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_93(uint start_byte_offset) {
  uvec4 v_94 = v.inner[(start_byte_offset / 16u)];
  vec2 v_95 = uintBitsToFloat(mix(v_94.xy, v_94.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_96 = (8u + start_byte_offset);
  uvec4 v_97 = v.inner[(v_96 / 16u)];
  vec2 v_98 = uintBitsToFloat(mix(v_97.xy, v_97.zw, bvec2((((v_96 & 15u) >> 2u) == 2u))));
  uint v_99 = (16u + start_byte_offset);
  uvec4 v_100 = v.inner[(v_99 / 16u)];
  return mat3x2(v_95, v_98, uintBitsToFloat(mix(v_100.xy, v_100.zw, bvec2((((v_99 & 15u) >> 2u) == 2u)))));
}
mat2x4 v_101(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_102(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_103(uint start_byte_offset) {
  uvec4 v_104 = v.inner[(start_byte_offset / 16u)];
  vec2 v_105 = uintBitsToFloat(mix(v_104.xy, v_104.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_106 = (8u + start_byte_offset);
  uvec4 v_107 = v.inner[(v_106 / 16u)];
  return mat2(v_105, uintBitsToFloat(mix(v_107.xy, v_107.zw, bvec2((((v_106 & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_108 = v.inner[0u];
  float scalar_f32 = uintBitsToFloat(v_108.x);
  uvec4 v_109 = v.inner[0u];
  int scalar_i32 = int(v_109.y);
  uvec4 v_110 = v.inner[0u];
  uint scalar_u32 = v_110.z;
  uvec4 v_111 = v.inner[0u];
  float16_t scalar_f16 = tint_bitcast_to_16bit(v_111.w).x;
  vec2 vec2_f32 = uintBitsToFloat(v.inner[1u].xy);
  ivec2 vec2_i32 = ivec2(v.inner[1u].zw);
  uvec2 vec2_u32 = v.inner[2u].xy;
  f16vec2 vec2_f16 = tint_bitcast_to_16bit(v.inner[2u].z);
  vec3 vec3_f32 = uintBitsToFloat(v.inner[3u].xyz);
  ivec3 vec3_i32 = ivec3(v.inner[4u].xyz);
  uvec3 vec3_u32 = v.inner[5u].xyz;
  f16vec3 vec3_f16 = tint_bitcast_to_16bit_1(v.inner[6u].xy).xyz;
  vec4 vec4_f32 = uintBitsToFloat(v.inner[7u]);
  ivec4 vec4_i32 = ivec4(v.inner[8u]);
  uvec4 vec4_u32 = v.inner[9u];
  f16vec4 vec4_f16 = tint_bitcast_to_16bit_1(v.inner[10u].xy);
  mat2 mat2x2_f32 = v_103(168u);
  mat2x3 mat2x3_f32 = v_102(192u);
  mat2x4 mat2x4_f32 = v_101(224u);
  mat3x2 mat3x2_f32 = v_93(256u);
  mat3 mat3x3_f32 = v_92(288u);
  mat3x4 mat3x4_f32 = v_91(336u);
  mat4x2 mat4x2_f32 = v_80(384u);
  mat4x3 mat4x3_f32 = v_79(416u);
  mat4 mat4x4_f32 = v_78(480u);
  f16mat2 mat2x2_f16 = v_75(544u);
  f16mat2x3 mat2x3_f16 = v_70(552u);
  f16mat2x4 mat2x4_f16 = v_65(568u);
  f16mat3x2 mat3x2_f16 = v_60(584u);
  f16mat3 mat3x3_f16 = v_52(600u);
  f16mat3x4 mat3x4_f16 = v_44(624u);
  f16mat4x2 mat4x2_f16 = v_11(648u);
  f16mat4x3 mat4x3_f16 = v_33(664u);
  f16mat4 mat4x4_f16 = v_22(696u);
  vec3 arr2_vec3_f32[2] = v_20(736u);
  f16mat4x2 arr2_mat4x2_f16[2] = v_18(768u);
  Inner struct_inner = v_2(800u);
  Inner array_struct_inner[4] = v_9(816u);
  uint v_112 = uint(tint_f32_to_i32(scalar_f32));
  int v_113 = int((v_112 + uint(scalar_i32)));
  int v_114 = int(scalar_u32);
  uint v_115 = uint(v_113);
  int v_116 = int((v_115 + uint(v_114)));
  int v_117 = tint_f16_to_i32(scalar_f16);
  uint v_118 = uint(v_116);
  int v_119 = int((v_118 + uint(v_117)));
  int v_120 = tint_f32_to_i32(vec2_f32.x);
  uint v_121 = uint(v_119);
  uint v_122 = uint(int((v_121 + uint(v_120))));
  int v_123 = int((v_122 + uint(vec2_i32.x)));
  int v_124 = int(vec2_u32.x);
  uint v_125 = uint(v_123);
  int v_126 = int((v_125 + uint(v_124)));
  int v_127 = tint_f16_to_i32(vec2_f16.x);
  uint v_128 = uint(v_126);
  int v_129 = int((v_128 + uint(v_127)));
  int v_130 = tint_f32_to_i32(vec3_f32.y);
  uint v_131 = uint(v_129);
  uint v_132 = uint(int((v_131 + uint(v_130))));
  int v_133 = int((v_132 + uint(vec3_i32.y)));
  int v_134 = int(vec3_u32.y);
  uint v_135 = uint(v_133);
  int v_136 = int((v_135 + uint(v_134)));
  int v_137 = tint_f16_to_i32(vec3_f16.y);
  uint v_138 = uint(v_136);
  int v_139 = int((v_138 + uint(v_137)));
  int v_140 = tint_f32_to_i32(vec4_f32.z);
  uint v_141 = uint(v_139);
  uint v_142 = uint(int((v_141 + uint(v_140))));
  int v_143 = int((v_142 + uint(vec4_i32.z)));
  int v_144 = int(vec4_u32.z);
  uint v_145 = uint(v_143);
  int v_146 = int((v_145 + uint(v_144)));
  int v_147 = tint_f16_to_i32(vec4_f16.z);
  uint v_148 = uint(v_146);
  int v_149 = int((v_148 + uint(v_147)));
  int v_150 = tint_f32_to_i32(mat2x2_f32[0].x);
  uint v_151 = uint(v_149);
  int v_152 = int((v_151 + uint(v_150)));
  int v_153 = tint_f32_to_i32(mat2x3_f32[0].x);
  uint v_154 = uint(v_152);
  int v_155 = int((v_154 + uint(v_153)));
  int v_156 = tint_f32_to_i32(mat2x4_f32[0].x);
  uint v_157 = uint(v_155);
  int v_158 = int((v_157 + uint(v_156)));
  int v_159 = tint_f32_to_i32(mat3x2_f32[0].x);
  uint v_160 = uint(v_158);
  int v_161 = int((v_160 + uint(v_159)));
  int v_162 = tint_f32_to_i32(mat3x3_f32[0].x);
  uint v_163 = uint(v_161);
  int v_164 = int((v_163 + uint(v_162)));
  int v_165 = tint_f32_to_i32(mat3x4_f32[0].x);
  uint v_166 = uint(v_164);
  int v_167 = int((v_166 + uint(v_165)));
  int v_168 = tint_f32_to_i32(mat4x2_f32[0].x);
  uint v_169 = uint(v_167);
  int v_170 = int((v_169 + uint(v_168)));
  int v_171 = tint_f32_to_i32(mat4x3_f32[0].x);
  uint v_172 = uint(v_170);
  int v_173 = int((v_172 + uint(v_171)));
  int v_174 = tint_f32_to_i32(mat4x4_f32[0].x);
  uint v_175 = uint(v_173);
  int v_176 = int((v_175 + uint(v_174)));
  int v_177 = tint_f16_to_i32(mat2x2_f16[0].x);
  uint v_178 = uint(v_176);
  int v_179 = int((v_178 + uint(v_177)));
  int v_180 = tint_f16_to_i32(mat2x3_f16[0].x);
  uint v_181 = uint(v_179);
  int v_182 = int((v_181 + uint(v_180)));
  int v_183 = tint_f16_to_i32(mat2x4_f16[0].x);
  uint v_184 = uint(v_182);
  int v_185 = int((v_184 + uint(v_183)));
  int v_186 = tint_f16_to_i32(mat3x2_f16[0].x);
  uint v_187 = uint(v_185);
  int v_188 = int((v_187 + uint(v_186)));
  int v_189 = tint_f16_to_i32(mat3x3_f16[0].x);
  uint v_190 = uint(v_188);
  int v_191 = int((v_190 + uint(v_189)));
  int v_192 = tint_f16_to_i32(mat3x4_f16[0].x);
  uint v_193 = uint(v_191);
  int v_194 = int((v_193 + uint(v_192)));
  int v_195 = tint_f16_to_i32(mat4x2_f16[0].x);
  uint v_196 = uint(v_194);
  int v_197 = int((v_196 + uint(v_195)));
  int v_198 = tint_f16_to_i32(mat4x3_f16[0].x);
  uint v_199 = uint(v_197);
  int v_200 = int((v_199 + uint(v_198)));
  int v_201 = tint_f16_to_i32(mat4x4_f16[0].x);
  uint v_202 = uint(v_200);
  int v_203 = int((v_202 + uint(v_201)));
  int v_204 = tint_f32_to_i32(arr2_vec3_f32[0].x);
  uint v_205 = uint(v_203);
  int v_206 = int((v_205 + uint(v_204)));
  int v_207 = tint_f16_to_i32(arr2_mat4x2_f16[0][0].x);
  uint v_208 = uint(v_206);
  uint v_209 = uint(int((v_208 + uint(v_207))));
  uint v_210 = uint(int((v_209 + uint(struct_inner.scalar_i32))));
  v_1.inner = int((v_210 + uint(array_struct_inner[0].scalar_i32)));
}
