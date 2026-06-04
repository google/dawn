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
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      a[v_11] = v_2((start_byte_offset + (v_11 * 16u)));
      {
        v_10 = (v_11 + 1u);
      }
    }
  }
  return a;
}
f16mat4x2 v_12(uint start_byte_offset) {
  f16vec2 v_13 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_14 = (4u + start_byte_offset);
  f16vec2 v_15 = tint_bitcast_to_16bit(v.inner[(v_14 / 16u)][((v_14 & 15u) >> 2u)]);
  uint v_16 = (8u + start_byte_offset);
  f16vec2 v_17 = tint_bitcast_to_16bit(v.inner[(v_16 / 16u)][((v_16 & 15u) >> 2u)]);
  uint v_18 = (12u + start_byte_offset);
  return f16mat4x2(v_13, v_15, v_17, tint_bitcast_to_16bit(v.inner[(v_18 / 16u)][((v_18 & 15u) >> 2u)]));
}
f16mat4x2[2] v_19(uint start_byte_offset) {
  f16mat4x2 a[2] = f16mat4x2[2](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint v_20 = 0u;
    v_20 = 0u;
    while(true) {
      uint v_21 = v_20;
      if ((v_21 >= 2u)) {
        break;
      }
      a[v_21] = v_12((start_byte_offset + (v_21 * 16u)));
      {
        v_20 = (v_21 + 1u);
      }
    }
  }
  return a;
}
vec3[2] v_22(uint start_byte_offset) {
  vec3 a[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  {
    uint v_23 = 0u;
    v_23 = 0u;
    while(true) {
      uint v_24 = v_23;
      if ((v_24 >= 2u)) {
        break;
      }
      a[v_24] = uintBitsToFloat(v.inner[((start_byte_offset + (v_24 * 16u)) / 16u)].xyz);
      {
        v_23 = (v_24 + 1u);
      }
    }
  }
  return a;
}
f16vec4 tint_bitcast_to_16bit_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_25(uint start_byte_offset) {
  uvec4 v_26 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_27 = tint_bitcast_to_16bit_1(mix(v_26.xy, v_26.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_28 = (8u + start_byte_offset);
  uvec4 v_29 = v.inner[(v_28 / 16u)];
  f16vec4 v_30 = tint_bitcast_to_16bit_1(mix(v_29.xy, v_29.zw, bvec2((((v_28 & 15u) >> 2u) == 2u))));
  uint v_31 = (16u + start_byte_offset);
  uvec4 v_32 = v.inner[(v_31 / 16u)];
  f16vec4 v_33 = tint_bitcast_to_16bit_1(mix(v_32.xy, v_32.zw, bvec2((((v_31 & 15u) >> 2u) == 2u))));
  uint v_34 = (24u + start_byte_offset);
  uvec4 v_35 = v.inner[(v_34 / 16u)];
  return f16mat4(v_27, v_30, v_33, tint_bitcast_to_16bit_1(mix(v_35.xy, v_35.zw, bvec2((((v_34 & 15u) >> 2u) == 2u)))));
}
f16mat4x3 v_36(uint start_byte_offset) {
  uvec4 v_37 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_38 = tint_bitcast_to_16bit_1(mix(v_37.xy, v_37.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_39 = (8u + start_byte_offset);
  uvec4 v_40 = v.inner[(v_39 / 16u)];
  f16vec3 v_41 = tint_bitcast_to_16bit_1(mix(v_40.xy, v_40.zw, bvec2((((v_39 & 15u) >> 2u) == 2u)))).xyz;
  uint v_42 = (16u + start_byte_offset);
  uvec4 v_43 = v.inner[(v_42 / 16u)];
  f16vec3 v_44 = tint_bitcast_to_16bit_1(mix(v_43.xy, v_43.zw, bvec2((((v_42 & 15u) >> 2u) == 2u)))).xyz;
  uint v_45 = (24u + start_byte_offset);
  uvec4 v_46 = v.inner[(v_45 / 16u)];
  return f16mat4x3(v_38, v_41, v_44, tint_bitcast_to_16bit_1(mix(v_46.xy, v_46.zw, bvec2((((v_45 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat3x4 v_47(uint start_byte_offset) {
  uvec4 v_48 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_49 = tint_bitcast_to_16bit_1(mix(v_48.xy, v_48.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_50 = (8u + start_byte_offset);
  uvec4 v_51 = v.inner[(v_50 / 16u)];
  f16vec4 v_52 = tint_bitcast_to_16bit_1(mix(v_51.xy, v_51.zw, bvec2((((v_50 & 15u) >> 2u) == 2u))));
  uint v_53 = (16u + start_byte_offset);
  uvec4 v_54 = v.inner[(v_53 / 16u)];
  return f16mat3x4(v_49, v_52, tint_bitcast_to_16bit_1(mix(v_54.xy, v_54.zw, bvec2((((v_53 & 15u) >> 2u) == 2u)))));
}
f16mat3 v_55(uint start_byte_offset) {
  uvec4 v_56 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_57 = tint_bitcast_to_16bit_1(mix(v_56.xy, v_56.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_58 = (8u + start_byte_offset);
  uvec4 v_59 = v.inner[(v_58 / 16u)];
  f16vec3 v_60 = tint_bitcast_to_16bit_1(mix(v_59.xy, v_59.zw, bvec2((((v_58 & 15u) >> 2u) == 2u)))).xyz;
  uint v_61 = (16u + start_byte_offset);
  uvec4 v_62 = v.inner[(v_61 / 16u)];
  return f16mat3(v_57, v_60, tint_bitcast_to_16bit_1(mix(v_62.xy, v_62.zw, bvec2((((v_61 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat3x2 v_63(uint start_byte_offset) {
  f16vec2 v_64 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_65 = (4u + start_byte_offset);
  f16vec2 v_66 = tint_bitcast_to_16bit(v.inner[(v_65 / 16u)][((v_65 & 15u) >> 2u)]);
  uint v_67 = (8u + start_byte_offset);
  return f16mat3x2(v_64, v_66, tint_bitcast_to_16bit(v.inner[(v_67 / 16u)][((v_67 & 15u) >> 2u)]));
}
f16mat2x4 v_68(uint start_byte_offset) {
  uvec4 v_69 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_70 = tint_bitcast_to_16bit_1(mix(v_69.xy, v_69.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_71 = (8u + start_byte_offset);
  uvec4 v_72 = v.inner[(v_71 / 16u)];
  return f16mat2x4(v_70, tint_bitcast_to_16bit_1(mix(v_72.xy, v_72.zw, bvec2((((v_71 & 15u) >> 2u) == 2u)))));
}
f16mat2x3 v_73(uint start_byte_offset) {
  uvec4 v_74 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_75 = tint_bitcast_to_16bit_1(mix(v_74.xy, v_74.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_76 = (8u + start_byte_offset);
  uvec4 v_77 = v.inner[(v_76 / 16u)];
  return f16mat2x3(v_75, tint_bitcast_to_16bit_1(mix(v_77.xy, v_77.zw, bvec2((((v_76 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat2 v_78(uint start_byte_offset) {
  f16vec2 v_79 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_80 = (4u + start_byte_offset);
  return f16mat2(v_79, tint_bitcast_to_16bit(v.inner[(v_80 / 16u)][((v_80 & 15u) >> 2u)]));
}
mat4 v_81(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_82(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_83(uint start_byte_offset) {
  uvec4 v_84 = v.inner[(start_byte_offset / 16u)];
  vec2 v_85 = uintBitsToFloat(mix(v_84.xy, v_84.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_86 = (8u + start_byte_offset);
  uvec4 v_87 = v.inner[(v_86 / 16u)];
  vec2 v_88 = uintBitsToFloat(mix(v_87.xy, v_87.zw, bvec2((((v_86 & 15u) >> 2u) == 2u))));
  uint v_89 = (16u + start_byte_offset);
  uvec4 v_90 = v.inner[(v_89 / 16u)];
  vec2 v_91 = uintBitsToFloat(mix(v_90.xy, v_90.zw, bvec2((((v_89 & 15u) >> 2u) == 2u))));
  uint v_92 = (24u + start_byte_offset);
  uvec4 v_93 = v.inner[(v_92 / 16u)];
  return mat4x2(v_85, v_88, v_91, uintBitsToFloat(mix(v_93.xy, v_93.zw, bvec2((((v_92 & 15u) >> 2u) == 2u)))));
}
mat3x4 v_94(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_95(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_96(uint start_byte_offset) {
  uvec4 v_97 = v.inner[(start_byte_offset / 16u)];
  vec2 v_98 = uintBitsToFloat(mix(v_97.xy, v_97.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_99 = (8u + start_byte_offset);
  uvec4 v_100 = v.inner[(v_99 / 16u)];
  vec2 v_101 = uintBitsToFloat(mix(v_100.xy, v_100.zw, bvec2((((v_99 & 15u) >> 2u) == 2u))));
  uint v_102 = (16u + start_byte_offset);
  uvec4 v_103 = v.inner[(v_102 / 16u)];
  return mat3x2(v_98, v_101, uintBitsToFloat(mix(v_103.xy, v_103.zw, bvec2((((v_102 & 15u) >> 2u) == 2u)))));
}
mat2x4 v_104(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_105(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_106(uint start_byte_offset) {
  uvec4 v_107 = v.inner[(start_byte_offset / 16u)];
  vec2 v_108 = uintBitsToFloat(mix(v_107.xy, v_107.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_109 = (8u + start_byte_offset);
  uvec4 v_110 = v.inner[(v_109 / 16u)];
  return mat2(v_108, uintBitsToFloat(mix(v_110.xy, v_110.zw, bvec2((((v_109 & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_111 = v.inner[0u];
  float scalar_f32 = uintBitsToFloat(v_111.x);
  uvec4 v_112 = v.inner[0u];
  int scalar_i32 = int(v_112.y);
  uvec4 v_113 = v.inner[0u];
  uint scalar_u32 = v_113.z;
  uvec4 v_114 = v.inner[0u];
  float16_t scalar_f16 = tint_bitcast_to_16bit(v_114.w).x;
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
  mat2 mat2x2_f32 = v_106(168u);
  mat2x3 mat2x3_f32 = v_105(192u);
  mat2x4 mat2x4_f32 = v_104(224u);
  mat3x2 mat3x2_f32 = v_96(256u);
  mat3 mat3x3_f32 = v_95(288u);
  mat3x4 mat3x4_f32 = v_94(336u);
  mat4x2 mat4x2_f32 = v_83(384u);
  mat4x3 mat4x3_f32 = v_82(416u);
  mat4 mat4x4_f32 = v_81(480u);
  f16mat2 mat2x2_f16 = v_78(544u);
  f16mat2x3 mat2x3_f16 = v_73(552u);
  f16mat2x4 mat2x4_f16 = v_68(568u);
  f16mat3x2 mat3x2_f16 = v_63(584u);
  f16mat3 mat3x3_f16 = v_55(600u);
  f16mat3x4 mat3x4_f16 = v_47(624u);
  f16mat4x2 mat4x2_f16 = v_12(648u);
  f16mat4x3 mat4x3_f16 = v_36(664u);
  f16mat4 mat4x4_f16 = v_25(696u);
  vec3 arr2_vec3_f32[2] = v_22(736u);
  f16mat4x2 arr2_mat4x2_f16[2] = v_19(768u);
  Inner struct_inner = v_2(800u);
  Inner array_struct_inner[4] = v_9(816u);
  uint v_115 = uint(tint_f32_to_i32(scalar_f32));
  int v_116 = int((v_115 + uint(scalar_i32)));
  int v_117 = int(scalar_u32);
  uint v_118 = uint(v_116);
  int v_119 = int((v_118 + uint(v_117)));
  int v_120 = tint_f16_to_i32(scalar_f16);
  uint v_121 = uint(v_119);
  int v_122 = int((v_121 + uint(v_120)));
  int v_123 = tint_f32_to_i32(vec2_f32.x);
  uint v_124 = uint(v_122);
  uint v_125 = uint(int((v_124 + uint(v_123))));
  int v_126 = int((v_125 + uint(vec2_i32.x)));
  int v_127 = int(vec2_u32.x);
  uint v_128 = uint(v_126);
  int v_129 = int((v_128 + uint(v_127)));
  int v_130 = tint_f16_to_i32(vec2_f16.x);
  uint v_131 = uint(v_129);
  int v_132 = int((v_131 + uint(v_130)));
  int v_133 = tint_f32_to_i32(vec3_f32.y);
  uint v_134 = uint(v_132);
  uint v_135 = uint(int((v_134 + uint(v_133))));
  int v_136 = int((v_135 + uint(vec3_i32.y)));
  int v_137 = int(vec3_u32.y);
  uint v_138 = uint(v_136);
  int v_139 = int((v_138 + uint(v_137)));
  int v_140 = tint_f16_to_i32(vec3_f16.y);
  uint v_141 = uint(v_139);
  int v_142 = int((v_141 + uint(v_140)));
  int v_143 = tint_f32_to_i32(vec4_f32.z);
  uint v_144 = uint(v_142);
  uint v_145 = uint(int((v_144 + uint(v_143))));
  int v_146 = int((v_145 + uint(vec4_i32.z)));
  int v_147 = int(vec4_u32.z);
  uint v_148 = uint(v_146);
  int v_149 = int((v_148 + uint(v_147)));
  int v_150 = tint_f16_to_i32(vec4_f16.z);
  uint v_151 = uint(v_149);
  int v_152 = int((v_151 + uint(v_150)));
  int v_153 = tint_f32_to_i32(mat2x2_f32[0u].x);
  uint v_154 = uint(v_152);
  int v_155 = int((v_154 + uint(v_153)));
  int v_156 = tint_f32_to_i32(mat2x3_f32[0u].x);
  uint v_157 = uint(v_155);
  int v_158 = int((v_157 + uint(v_156)));
  int v_159 = tint_f32_to_i32(mat2x4_f32[0u].x);
  uint v_160 = uint(v_158);
  int v_161 = int((v_160 + uint(v_159)));
  int v_162 = tint_f32_to_i32(mat3x2_f32[0u].x);
  uint v_163 = uint(v_161);
  int v_164 = int((v_163 + uint(v_162)));
  int v_165 = tint_f32_to_i32(mat3x3_f32[0u].x);
  uint v_166 = uint(v_164);
  int v_167 = int((v_166 + uint(v_165)));
  int v_168 = tint_f32_to_i32(mat3x4_f32[0u].x);
  uint v_169 = uint(v_167);
  int v_170 = int((v_169 + uint(v_168)));
  int v_171 = tint_f32_to_i32(mat4x2_f32[0u].x);
  uint v_172 = uint(v_170);
  int v_173 = int((v_172 + uint(v_171)));
  int v_174 = tint_f32_to_i32(mat4x3_f32[0u].x);
  uint v_175 = uint(v_173);
  int v_176 = int((v_175 + uint(v_174)));
  int v_177 = tint_f32_to_i32(mat4x4_f32[0u].x);
  uint v_178 = uint(v_176);
  int v_179 = int((v_178 + uint(v_177)));
  int v_180 = tint_f16_to_i32(mat2x2_f16[0u].x);
  uint v_181 = uint(v_179);
  int v_182 = int((v_181 + uint(v_180)));
  int v_183 = tint_f16_to_i32(mat2x3_f16[0u].x);
  uint v_184 = uint(v_182);
  int v_185 = int((v_184 + uint(v_183)));
  int v_186 = tint_f16_to_i32(mat2x4_f16[0u].x);
  uint v_187 = uint(v_185);
  int v_188 = int((v_187 + uint(v_186)));
  int v_189 = tint_f16_to_i32(mat3x2_f16[0u].x);
  uint v_190 = uint(v_188);
  int v_191 = int((v_190 + uint(v_189)));
  int v_192 = tint_f16_to_i32(mat3x3_f16[0u].x);
  uint v_193 = uint(v_191);
  int v_194 = int((v_193 + uint(v_192)));
  int v_195 = tint_f16_to_i32(mat3x4_f16[0u].x);
  uint v_196 = uint(v_194);
  int v_197 = int((v_196 + uint(v_195)));
  int v_198 = tint_f16_to_i32(mat4x2_f16[0u].x);
  uint v_199 = uint(v_197);
  int v_200 = int((v_199 + uint(v_198)));
  int v_201 = tint_f16_to_i32(mat4x3_f16[0u].x);
  uint v_202 = uint(v_200);
  int v_203 = int((v_202 + uint(v_201)));
  int v_204 = tint_f16_to_i32(mat4x4_f16[0u].x);
  uint v_205 = uint(v_203);
  int v_206 = int((v_205 + uint(v_204)));
  int v_207 = tint_f32_to_i32(arr2_vec3_f32[0u].x);
  uint v_208 = uint(v_206);
  int v_209 = int((v_208 + uint(v_207)));
  int v_210 = tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x);
  uint v_211 = uint(v_209);
  uint v_212 = uint(int((v_211 + uint(v_210))));
  uint v_213 = uint(int((v_212 + uint(struct_inner.scalar_i32))));
  v_1.inner = int((v_213 + uint(array_struct_inner[0u].scalar_i32)));
}
