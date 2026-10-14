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
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  f16vec2 v_5 = tint_bitcast_to_16bit(v.inner[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  f16vec2 v_7 = tint_bitcast_to_16bit(v.inner[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return f16mat4x2(v_3, v_5, v_7, tint_bitcast_to_16bit(v.inner[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}
f16mat4x2[2] v_9(uint start_byte_offset) {
  f16mat4x2 a[2] = f16mat4x2[2](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint idx = 0u;
    while(true) {
      uint v_10 = idx;
      if ((v_10 >= 2u)) {
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
vec3[2] v_11(uint start_byte_offset) {
  vec3 a[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 2u)) {
        break;
      }
      a[v_12] = uintBitsToFloat(v.inner[((start_byte_offset + (v_12 * 16u)) / 16u)].xyz);
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
f16mat4 v_13(uint start_byte_offset) {
  uvec4 v_14 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_15 = tint_bitcast_to_16bit_1(mix(v_14.xy, v_14.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_16 = (8u + start_byte_offset);
  uvec4 v_17 = v.inner[(v_16 / 16u)];
  f16vec4 v_18 = tint_bitcast_to_16bit_1(mix(v_17.xy, v_17.zw, bvec2((((v_16 & 15u) >> 2u) == 2u))));
  uint v_19 = (16u + start_byte_offset);
  uvec4 v_20 = v.inner[(v_19 / 16u)];
  f16vec4 v_21 = tint_bitcast_to_16bit_1(mix(v_20.xy, v_20.zw, bvec2((((v_19 & 15u) >> 2u) == 2u))));
  uint v_22 = (24u + start_byte_offset);
  uvec4 v_23 = v.inner[(v_22 / 16u)];
  return f16mat4(v_15, v_18, v_21, tint_bitcast_to_16bit_1(mix(v_23.xy, v_23.zw, bvec2((((v_22 & 15u) >> 2u) == 2u)))));
}
f16mat4x3 v_24(uint start_byte_offset) {
  uvec4 v_25 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_26 = tint_bitcast_to_16bit_1(mix(v_25.xy, v_25.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_27 = (8u + start_byte_offset);
  uvec4 v_28 = v.inner[(v_27 / 16u)];
  f16vec3 v_29 = tint_bitcast_to_16bit_1(mix(v_28.xy, v_28.zw, bvec2((((v_27 & 15u) >> 2u) == 2u)))).xyz;
  uint v_30 = (16u + start_byte_offset);
  uvec4 v_31 = v.inner[(v_30 / 16u)];
  f16vec3 v_32 = tint_bitcast_to_16bit_1(mix(v_31.xy, v_31.zw, bvec2((((v_30 & 15u) >> 2u) == 2u)))).xyz;
  uint v_33 = (24u + start_byte_offset);
  uvec4 v_34 = v.inner[(v_33 / 16u)];
  return f16mat4x3(v_26, v_29, v_32, tint_bitcast_to_16bit_1(mix(v_34.xy, v_34.zw, bvec2((((v_33 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat3x4 v_35(uint start_byte_offset) {
  uvec4 v_36 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_37 = tint_bitcast_to_16bit_1(mix(v_36.xy, v_36.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_38 = (8u + start_byte_offset);
  uvec4 v_39 = v.inner[(v_38 / 16u)];
  f16vec4 v_40 = tint_bitcast_to_16bit_1(mix(v_39.xy, v_39.zw, bvec2((((v_38 & 15u) >> 2u) == 2u))));
  uint v_41 = (16u + start_byte_offset);
  uvec4 v_42 = v.inner[(v_41 / 16u)];
  return f16mat3x4(v_37, v_40, tint_bitcast_to_16bit_1(mix(v_42.xy, v_42.zw, bvec2((((v_41 & 15u) >> 2u) == 2u)))));
}
f16mat3 v_43(uint start_byte_offset) {
  uvec4 v_44 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_45 = tint_bitcast_to_16bit_1(mix(v_44.xy, v_44.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_46 = (8u + start_byte_offset);
  uvec4 v_47 = v.inner[(v_46 / 16u)];
  f16vec3 v_48 = tint_bitcast_to_16bit_1(mix(v_47.xy, v_47.zw, bvec2((((v_46 & 15u) >> 2u) == 2u)))).xyz;
  uint v_49 = (16u + start_byte_offset);
  uvec4 v_50 = v.inner[(v_49 / 16u)];
  return f16mat3(v_45, v_48, tint_bitcast_to_16bit_1(mix(v_50.xy, v_50.zw, bvec2((((v_49 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat3x2 v_51(uint start_byte_offset) {
  f16vec2 v_52 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_53 = (4u + start_byte_offset);
  f16vec2 v_54 = tint_bitcast_to_16bit(v.inner[(v_53 / 16u)][((v_53 & 15u) >> 2u)]);
  uint v_55 = (8u + start_byte_offset);
  return f16mat3x2(v_52, v_54, tint_bitcast_to_16bit(v.inner[(v_55 / 16u)][((v_55 & 15u) >> 2u)]));
}
f16mat2x4 v_56(uint start_byte_offset) {
  uvec4 v_57 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_58 = tint_bitcast_to_16bit_1(mix(v_57.xy, v_57.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_59 = (8u + start_byte_offset);
  uvec4 v_60 = v.inner[(v_59 / 16u)];
  return f16mat2x4(v_58, tint_bitcast_to_16bit_1(mix(v_60.xy, v_60.zw, bvec2((((v_59 & 15u) >> 2u) == 2u)))));
}
f16mat2x3 v_61(uint start_byte_offset) {
  uvec4 v_62 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_63 = tint_bitcast_to_16bit_1(mix(v_62.xy, v_62.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_64 = (8u + start_byte_offset);
  uvec4 v_65 = v.inner[(v_64 / 16u)];
  return f16mat2x3(v_63, tint_bitcast_to_16bit_1(mix(v_65.xy, v_65.zw, bvec2((((v_64 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat2 v_66(uint start_byte_offset) {
  f16vec2 v_67 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_68 = (4u + start_byte_offset);
  return f16mat2(v_67, tint_bitcast_to_16bit(v.inner[(v_68 / 16u)][((v_68 & 15u) >> 2u)]));
}
mat4 v_69(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_70(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_71(uint start_byte_offset) {
  uvec4 v_72 = v.inner[(start_byte_offset / 16u)];
  vec2 v_73 = uintBitsToFloat(mix(v_72.xy, v_72.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_74 = (8u + start_byte_offset);
  uvec4 v_75 = v.inner[(v_74 / 16u)];
  vec2 v_76 = uintBitsToFloat(mix(v_75.xy, v_75.zw, bvec2((((v_74 & 15u) >> 2u) == 2u))));
  uint v_77 = (16u + start_byte_offset);
  uvec4 v_78 = v.inner[(v_77 / 16u)];
  vec2 v_79 = uintBitsToFloat(mix(v_78.xy, v_78.zw, bvec2((((v_77 & 15u) >> 2u) == 2u))));
  uint v_80 = (24u + start_byte_offset);
  uvec4 v_81 = v.inner[(v_80 / 16u)];
  return mat4x2(v_73, v_76, v_79, uintBitsToFloat(mix(v_81.xy, v_81.zw, bvec2((((v_80 & 15u) >> 2u) == 2u)))));
}
mat3x4 v_82(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_83(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_84(uint start_byte_offset) {
  uvec4 v_85 = v.inner[(start_byte_offset / 16u)];
  vec2 v_86 = uintBitsToFloat(mix(v_85.xy, v_85.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_87 = (8u + start_byte_offset);
  uvec4 v_88 = v.inner[(v_87 / 16u)];
  vec2 v_89 = uintBitsToFloat(mix(v_88.xy, v_88.zw, bvec2((((v_87 & 15u) >> 2u) == 2u))));
  uint v_90 = (16u + start_byte_offset);
  uvec4 v_91 = v.inner[(v_90 / 16u)];
  return mat3x2(v_86, v_89, uintBitsToFloat(mix(v_91.xy, v_91.zw, bvec2((((v_90 & 15u) >> 2u) == 2u)))));
}
mat2x4 v_92(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_93(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_94(uint start_byte_offset) {
  uvec4 v_95 = v.inner[(start_byte_offset / 16u)];
  vec2 v_96 = uintBitsToFloat(mix(v_95.xy, v_95.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_97 = (8u + start_byte_offset);
  uvec4 v_98 = v.inner[(v_97 / 16u)];
  return mat2(v_96, uintBitsToFloat(mix(v_98.xy, v_98.zw, bvec2((((v_97 & 15u) >> 2u) == 2u)))));
}
void main_inner(uint idx) {
  uint v_99 = (idx * 800u);
  uvec4 v_100 = v.inner[(v_99 / 16u)];
  float scalar_f32 = uintBitsToFloat(v_100[((v_99 & 15u) >> 2u)]);
  uint v_101 = (4u + (idx * 800u));
  uvec4 v_102 = v.inner[(v_101 / 16u)];
  int scalar_i32 = int(v_102[((v_101 & 15u) >> 2u)]);
  uint v_103 = (8u + (idx * 800u));
  uvec4 v_104 = v.inner[(v_103 / 16u)];
  uint scalar_u32 = v_104[((v_103 & 15u) >> 2u)];
  uint v_105 = (12u + (idx * 800u));
  uvec4 v_106 = v.inner[(v_105 / 16u)];
  float16_t scalar_f16 = tint_bitcast_to_16bit(v_106[((v_105 & 15u) >> 2u)])[mix(1u, 0u, ((v_105 % 4u) == 0u))];
  uint v_107 = (16u + (idx * 800u));
  uvec4 v_108 = v.inner[(v_107 / 16u)];
  vec2 vec2_f32 = uintBitsToFloat(mix(v_108.xy, v_108.zw, bvec2((((v_107 & 15u) >> 2u) == 2u))));
  uint v_109 = (24u + (idx * 800u));
  uvec4 v_110 = v.inner[(v_109 / 16u)];
  ivec2 vec2_i32 = ivec2(mix(v_110.xy, v_110.zw, bvec2((((v_109 & 15u) >> 2u) == 2u))));
  uint v_111 = (32u + (idx * 800u));
  uvec4 v_112 = v.inner[(v_111 / 16u)];
  uvec2 vec2_u32 = mix(v_112.xy, v_112.zw, bvec2((((v_111 & 15u) >> 2u) == 2u)));
  uint v_113 = (40u + (idx * 800u));
  f16vec2 vec2_f16 = tint_bitcast_to_16bit(v.inner[(v_113 / 16u)][((v_113 & 15u) >> 2u)]);
  vec3 vec3_f32 = uintBitsToFloat(v.inner[((48u + (idx * 800u)) / 16u)].xyz);
  ivec3 vec3_i32 = ivec3(v.inner[((64u + (idx * 800u)) / 16u)].xyz);
  uvec3 vec3_u32 = v.inner[((80u + (idx * 800u)) / 16u)].xyz;
  uint v_114 = (96u + (idx * 800u));
  uvec4 v_115 = v.inner[(v_114 / 16u)];
  f16vec3 vec3_f16 = tint_bitcast_to_16bit_1(mix(v_115.xy, v_115.zw, bvec2((((v_114 & 15u) >> 2u) == 2u)))).xyz;
  vec4 vec4_f32 = uintBitsToFloat(v.inner[((112u + (idx * 800u)) / 16u)]);
  ivec4 vec4_i32 = ivec4(v.inner[((128u + (idx * 800u)) / 16u)]);
  uvec4 vec4_u32 = v.inner[((144u + (idx * 800u)) / 16u)];
  uint v_116 = (160u + (idx * 800u));
  uvec4 v_117 = v.inner[(v_116 / 16u)];
  f16vec4 vec4_f16 = tint_bitcast_to_16bit_1(mix(v_117.xy, v_117.zw, bvec2((((v_116 & 15u) >> 2u) == 2u))));
  mat2 mat2x2_f32 = v_94((168u + (idx * 800u)));
  mat2x3 mat2x3_f32 = v_93((192u + (idx * 800u)));
  mat2x4 mat2x4_f32 = v_92((224u + (idx * 800u)));
  mat3x2 mat3x2_f32 = v_84((256u + (idx * 800u)));
  mat3 mat3x3_f32 = v_83((288u + (idx * 800u)));
  mat3x4 mat3x4_f32 = v_82((336u + (idx * 800u)));
  mat4x2 mat4x2_f32 = v_71((384u + (idx * 800u)));
  mat4x3 mat4x3_f32 = v_70((416u + (idx * 800u)));
  mat4 mat4x4_f32 = v_69((480u + (idx * 800u)));
  f16mat2 mat2x2_f16 = v_66((544u + (idx * 800u)));
  f16mat2x3 mat2x3_f16 = v_61((552u + (idx * 800u)));
  f16mat2x4 mat2x4_f16 = v_56((568u + (idx * 800u)));
  f16mat3x2 mat3x2_f16 = v_51((584u + (idx * 800u)));
  f16mat3 mat3x3_f16 = v_43((600u + (idx * 800u)));
  f16mat3x4 mat3x4_f16 = v_35((624u + (idx * 800u)));
  f16mat4x2 mat4x2_f16 = v_2((648u + (idx * 800u)));
  f16mat4x3 mat4x3_f16 = v_24((664u + (idx * 800u)));
  f16mat4 mat4x4_f16 = v_13((696u + (idx * 800u)));
  vec3 arr2_vec3_f32[2] = v_11((736u + (idx * 800u)));
  f16mat4x2 arr2_mat4x2_f16[2] = v_9((768u + (idx * 800u)));
  uint v_118 = uint(tint_f32_to_i32(scalar_f32));
  int v_119 = int((v_118 + uint(scalar_i32)));
  int v_120 = int(scalar_u32);
  uint v_121 = uint(v_119);
  int v_122 = int((v_121 + uint(v_120)));
  int v_123 = tint_f16_to_i32(scalar_f16);
  uint v_124 = uint(v_122);
  int v_125 = int((v_124 + uint(v_123)));
  int v_126 = tint_f32_to_i32(vec2_f32.x);
  uint v_127 = uint(v_125);
  uint v_128 = uint(int((v_127 + uint(v_126))));
  int v_129 = int((v_128 + uint(vec2_i32.x)));
  int v_130 = int(vec2_u32.x);
  uint v_131 = uint(v_129);
  int v_132 = int((v_131 + uint(v_130)));
  int v_133 = tint_f16_to_i32(vec2_f16.x);
  uint v_134 = uint(v_132);
  int v_135 = int((v_134 + uint(v_133)));
  int v_136 = tint_f32_to_i32(vec3_f32.y);
  uint v_137 = uint(v_135);
  uint v_138 = uint(int((v_137 + uint(v_136))));
  int v_139 = int((v_138 + uint(vec3_i32.y)));
  int v_140 = int(vec3_u32.y);
  uint v_141 = uint(v_139);
  int v_142 = int((v_141 + uint(v_140)));
  int v_143 = tint_f16_to_i32(vec3_f16.y);
  uint v_144 = uint(v_142);
  int v_145 = int((v_144 + uint(v_143)));
  int v_146 = tint_f32_to_i32(vec4_f32.z);
  uint v_147 = uint(v_145);
  uint v_148 = uint(int((v_147 + uint(v_146))));
  int v_149 = int((v_148 + uint(vec4_i32.z)));
  int v_150 = int(vec4_u32.z);
  uint v_151 = uint(v_149);
  int v_152 = int((v_151 + uint(v_150)));
  int v_153 = tint_f16_to_i32(vec4_f16.z);
  uint v_154 = uint(v_152);
  int v_155 = int((v_154 + uint(v_153)));
  int v_156 = tint_f32_to_i32(mat2x2_f32[0].x);
  uint v_157 = uint(v_155);
  int v_158 = int((v_157 + uint(v_156)));
  int v_159 = tint_f32_to_i32(mat2x3_f32[0].x);
  uint v_160 = uint(v_158);
  int v_161 = int((v_160 + uint(v_159)));
  int v_162 = tint_f32_to_i32(mat2x4_f32[0].x);
  uint v_163 = uint(v_161);
  int v_164 = int((v_163 + uint(v_162)));
  int v_165 = tint_f32_to_i32(mat3x2_f32[0].x);
  uint v_166 = uint(v_164);
  int v_167 = int((v_166 + uint(v_165)));
  int v_168 = tint_f32_to_i32(mat3x3_f32[0].x);
  uint v_169 = uint(v_167);
  int v_170 = int((v_169 + uint(v_168)));
  int v_171 = tint_f32_to_i32(mat3x4_f32[0].x);
  uint v_172 = uint(v_170);
  int v_173 = int((v_172 + uint(v_171)));
  int v_174 = tint_f32_to_i32(mat4x2_f32[0].x);
  uint v_175 = uint(v_173);
  int v_176 = int((v_175 + uint(v_174)));
  int v_177 = tint_f32_to_i32(mat4x3_f32[0].x);
  uint v_178 = uint(v_176);
  int v_179 = int((v_178 + uint(v_177)));
  int v_180 = tint_f32_to_i32(mat4x4_f32[0].x);
  uint v_181 = uint(v_179);
  int v_182 = int((v_181 + uint(v_180)));
  int v_183 = tint_f16_to_i32(mat2x2_f16[0].x);
  uint v_184 = uint(v_182);
  int v_185 = int((v_184 + uint(v_183)));
  int v_186 = tint_f16_to_i32(mat2x3_f16[0].x);
  uint v_187 = uint(v_185);
  int v_188 = int((v_187 + uint(v_186)));
  int v_189 = tint_f16_to_i32(mat2x4_f16[0].x);
  uint v_190 = uint(v_188);
  int v_191 = int((v_190 + uint(v_189)));
  int v_192 = tint_f16_to_i32(mat3x2_f16[0].x);
  uint v_193 = uint(v_191);
  int v_194 = int((v_193 + uint(v_192)));
  int v_195 = tint_f16_to_i32(mat3x3_f16[0].x);
  uint v_196 = uint(v_194);
  int v_197 = int((v_196 + uint(v_195)));
  int v_198 = tint_f16_to_i32(mat3x4_f16[0].x);
  uint v_199 = uint(v_197);
  int v_200 = int((v_199 + uint(v_198)));
  int v_201 = tint_f16_to_i32(mat4x2_f16[0].x);
  uint v_202 = uint(v_200);
  int v_203 = int((v_202 + uint(v_201)));
  int v_204 = tint_f16_to_i32(mat4x3_f16[0].x);
  uint v_205 = uint(v_203);
  int v_206 = int((v_205 + uint(v_204)));
  int v_207 = tint_f16_to_i32(mat4x4_f16[0].x);
  uint v_208 = uint(v_206);
  int v_209 = int((v_208 + uint(v_207)));
  int v_210 = tint_f32_to_i32(arr2_vec3_f32[0].x);
  uint v_211 = uint(v_209);
  int v_212 = int((v_211 + uint(v_210)));
  int v_213 = tint_f16_to_i32(arr2_mat4x2_f16[0][0].x);
  uint v_214 = uint(v_212);
  v_1.inner = int((v_214 + uint(v_213)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
