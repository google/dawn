#version 310 es

layout(binding = 0, std140)
uniform ub_block_1_ubo {
  uvec4 inner[272];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  int inner;
} v_1;
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
vec3[2] v_2(uint start_byte_offset) {
  vec3 a[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 2u)) {
        break;
      }
      a[v_3] = uintBitsToFloat(v.inner[((start_byte_offset + (v_3 * 16u)) / 16u)].xyz);
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
mat4 v_4(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_5(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_6(uint start_byte_offset) {
  uvec4 v_7 = v.inner[(start_byte_offset / 16u)];
  vec2 v_8 = uintBitsToFloat(mix(v_7.xy, v_7.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_9 = (8u + start_byte_offset);
  uvec4 v_10 = v.inner[(v_9 / 16u)];
  vec2 v_11 = uintBitsToFloat(mix(v_10.xy, v_10.zw, bvec2((((v_9 & 15u) >> 2u) == 2u))));
  uint v_12 = (16u + start_byte_offset);
  uvec4 v_13 = v.inner[(v_12 / 16u)];
  vec2 v_14 = uintBitsToFloat(mix(v_13.xy, v_13.zw, bvec2((((v_12 & 15u) >> 2u) == 2u))));
  uint v_15 = (24u + start_byte_offset);
  uvec4 v_16 = v.inner[(v_15 / 16u)];
  return mat4x2(v_8, v_11, v_14, uintBitsToFloat(mix(v_16.xy, v_16.zw, bvec2((((v_15 & 15u) >> 2u) == 2u)))));
}
mat3x4 v_17(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_18(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_19(uint start_byte_offset) {
  uvec4 v_20 = v.inner[(start_byte_offset / 16u)];
  vec2 v_21 = uintBitsToFloat(mix(v_20.xy, v_20.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_22 = (8u + start_byte_offset);
  uvec4 v_23 = v.inner[(v_22 / 16u)];
  vec2 v_24 = uintBitsToFloat(mix(v_23.xy, v_23.zw, bvec2((((v_22 & 15u) >> 2u) == 2u))));
  uint v_25 = (16u + start_byte_offset);
  uvec4 v_26 = v.inner[(v_25 / 16u)];
  return mat3x2(v_21, v_24, uintBitsToFloat(mix(v_26.xy, v_26.zw, bvec2((((v_25 & 15u) >> 2u) == 2u)))));
}
mat2x4 v_27(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_28(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_29(uint start_byte_offset) {
  uvec4 v_30 = v.inner[(start_byte_offset / 16u)];
  vec2 v_31 = uintBitsToFloat(mix(v_30.xy, v_30.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_32 = (8u + start_byte_offset);
  uvec4 v_33 = v.inner[(v_32 / 16u)];
  return mat2(v_31, uintBitsToFloat(mix(v_33.xy, v_33.zw, bvec2((((v_32 & 15u) >> 2u) == 2u)))));
}
void main_inner(uint idx) {
  uint v_34 = (idx * 544u);
  uvec4 v_35 = v.inner[(v_34 / 16u)];
  float scalar_f32 = uintBitsToFloat(v_35[((v_34 & 15u) >> 2u)]);
  uint v_36 = (4u + (idx * 544u));
  uvec4 v_37 = v.inner[(v_36 / 16u)];
  int scalar_i32 = int(v_37[((v_36 & 15u) >> 2u)]);
  uint v_38 = (8u + (idx * 544u));
  uvec4 v_39 = v.inner[(v_38 / 16u)];
  uint scalar_u32 = v_39[((v_38 & 15u) >> 2u)];
  uint v_40 = (16u + (idx * 544u));
  uvec4 v_41 = v.inner[(v_40 / 16u)];
  vec2 vec2_f32 = uintBitsToFloat(mix(v_41.xy, v_41.zw, bvec2((((v_40 & 15u) >> 2u) == 2u))));
  uint v_42 = (24u + (idx * 544u));
  uvec4 v_43 = v.inner[(v_42 / 16u)];
  ivec2 vec2_i32 = ivec2(mix(v_43.xy, v_43.zw, bvec2((((v_42 & 15u) >> 2u) == 2u))));
  uint v_44 = (32u + (idx * 544u));
  uvec4 v_45 = v.inner[(v_44 / 16u)];
  uvec2 vec2_u32 = mix(v_45.xy, v_45.zw, bvec2((((v_44 & 15u) >> 2u) == 2u)));
  vec3 vec3_f32 = uintBitsToFloat(v.inner[((48u + (idx * 544u)) / 16u)].xyz);
  ivec3 vec3_i32 = ivec3(v.inner[((64u + (idx * 544u)) / 16u)].xyz);
  uvec3 vec3_u32 = v.inner[((80u + (idx * 544u)) / 16u)].xyz;
  vec4 vec4_f32 = uintBitsToFloat(v.inner[((96u + (idx * 544u)) / 16u)]);
  ivec4 vec4_i32 = ivec4(v.inner[((112u + (idx * 544u)) / 16u)]);
  uvec4 vec4_u32 = v.inner[((128u + (idx * 544u)) / 16u)];
  mat2 mat2x2_f32 = v_29((144u + (idx * 544u)));
  mat2x3 mat2x3_f32 = v_28((160u + (idx * 544u)));
  mat2x4 mat2x4_f32 = v_27((192u + (idx * 544u)));
  mat3x2 mat3x2_f32 = v_19((224u + (idx * 544u)));
  mat3 mat3x3_f32 = v_18((256u + (idx * 544u)));
  mat3x4 mat3x4_f32 = v_17((304u + (idx * 544u)));
  mat4x2 mat4x2_f32 = v_6((352u + (idx * 544u)));
  mat4x3 mat4x3_f32 = v_5((384u + (idx * 544u)));
  mat4 mat4x4_f32 = v_4((448u + (idx * 544u)));
  vec3 arr2_vec3_f32[2] = v_2((512u + (idx * 544u)));
  uint v_46 = uint(tint_f32_to_i32(scalar_f32));
  int v_47 = int((v_46 + uint(scalar_i32)));
  int v_48 = int(scalar_u32);
  uint v_49 = uint(v_47);
  int v_50 = int((v_49 + uint(v_48)));
  int v_51 = tint_f32_to_i32(vec2_f32.x);
  uint v_52 = uint(v_50);
  uint v_53 = uint(int((v_52 + uint(v_51))));
  int v_54 = int((v_53 + uint(vec2_i32.x)));
  int v_55 = int(vec2_u32.x);
  uint v_56 = uint(v_54);
  int v_57 = int((v_56 + uint(v_55)));
  int v_58 = tint_f32_to_i32(vec3_f32.y);
  uint v_59 = uint(v_57);
  uint v_60 = uint(int((v_59 + uint(v_58))));
  int v_61 = int((v_60 + uint(vec3_i32.y)));
  int v_62 = int(vec3_u32.y);
  uint v_63 = uint(v_61);
  int v_64 = int((v_63 + uint(v_62)));
  int v_65 = tint_f32_to_i32(vec4_f32.z);
  uint v_66 = uint(v_64);
  uint v_67 = uint(int((v_66 + uint(v_65))));
  int v_68 = int((v_67 + uint(vec4_i32.z)));
  int v_69 = int(vec4_u32.z);
  uint v_70 = uint(v_68);
  int v_71 = int((v_70 + uint(v_69)));
  int v_72 = tint_f32_to_i32(mat2x2_f32[0].x);
  uint v_73 = uint(v_71);
  int v_74 = int((v_73 + uint(v_72)));
  int v_75 = tint_f32_to_i32(mat2x3_f32[0].x);
  uint v_76 = uint(v_74);
  int v_77 = int((v_76 + uint(v_75)));
  int v_78 = tint_f32_to_i32(mat2x4_f32[0].x);
  uint v_79 = uint(v_77);
  int v_80 = int((v_79 + uint(v_78)));
  int v_81 = tint_f32_to_i32(mat3x2_f32[0].x);
  uint v_82 = uint(v_80);
  int v_83 = int((v_82 + uint(v_81)));
  int v_84 = tint_f32_to_i32(mat3x3_f32[0].x);
  uint v_85 = uint(v_83);
  int v_86 = int((v_85 + uint(v_84)));
  int v_87 = tint_f32_to_i32(mat3x4_f32[0].x);
  uint v_88 = uint(v_86);
  int v_89 = int((v_88 + uint(v_87)));
  int v_90 = tint_f32_to_i32(mat4x2_f32[0].x);
  uint v_91 = uint(v_89);
  int v_92 = int((v_91 + uint(v_90)));
  int v_93 = tint_f32_to_i32(mat4x3_f32[0].x);
  uint v_94 = uint(v_92);
  int v_95 = int((v_94 + uint(v_93)));
  int v_96 = tint_f32_to_i32(mat4x4_f32[0].x);
  uint v_97 = uint(v_95);
  int v_98 = int((v_97 + uint(v_96)));
  int v_99 = tint_f32_to_i32(arr2_vec3_f32[0].x);
  uint v_100 = uint(v_98);
  v_1.inner = int((v_100 + uint(v_99)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
