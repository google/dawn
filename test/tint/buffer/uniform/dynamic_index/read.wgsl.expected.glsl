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
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 2u)) {
        break;
      }
      a[v_4] = uintBitsToFloat(v.inner[((start_byte_offset + (v_4 * 16u)) / 16u)].xyz);
      {
        v_3 = (v_4 + 1u);
      }
    }
  }
  return a;
}
mat4 v_5(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_6(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_7(uint start_byte_offset) {
  uvec4 v_8 = v.inner[(start_byte_offset / 16u)];
  vec2 v_9 = uintBitsToFloat(mix(v_8.xy, v_8.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_10 = (8u + start_byte_offset);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  vec2 v_12 = uintBitsToFloat(mix(v_11.xy, v_11.zw, bvec2((((v_10 & 15u) >> 2u) == 2u))));
  uint v_13 = (16u + start_byte_offset);
  uvec4 v_14 = v.inner[(v_13 / 16u)];
  vec2 v_15 = uintBitsToFloat(mix(v_14.xy, v_14.zw, bvec2((((v_13 & 15u) >> 2u) == 2u))));
  uint v_16 = (24u + start_byte_offset);
  uvec4 v_17 = v.inner[(v_16 / 16u)];
  return mat4x2(v_9, v_12, v_15, uintBitsToFloat(mix(v_17.xy, v_17.zw, bvec2((((v_16 & 15u) >> 2u) == 2u)))));
}
mat3x4 v_18(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_19(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_20(uint start_byte_offset) {
  uvec4 v_21 = v.inner[(start_byte_offset / 16u)];
  vec2 v_22 = uintBitsToFloat(mix(v_21.xy, v_21.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_23 = (8u + start_byte_offset);
  uvec4 v_24 = v.inner[(v_23 / 16u)];
  vec2 v_25 = uintBitsToFloat(mix(v_24.xy, v_24.zw, bvec2((((v_23 & 15u) >> 2u) == 2u))));
  uint v_26 = (16u + start_byte_offset);
  uvec4 v_27 = v.inner[(v_26 / 16u)];
  return mat3x2(v_22, v_25, uintBitsToFloat(mix(v_27.xy, v_27.zw, bvec2((((v_26 & 15u) >> 2u) == 2u)))));
}
mat2x4 v_28(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_29(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_30(uint start_byte_offset) {
  uvec4 v_31 = v.inner[(start_byte_offset / 16u)];
  vec2 v_32 = uintBitsToFloat(mix(v_31.xy, v_31.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_33 = (8u + start_byte_offset);
  uvec4 v_34 = v.inner[(v_33 / 16u)];
  return mat2(v_32, uintBitsToFloat(mix(v_34.xy, v_34.zw, bvec2((((v_33 & 15u) >> 2u) == 2u)))));
}
void main_inner(uint idx) {
  uint v_35 = (idx * 544u);
  uvec4 v_36 = v.inner[(v_35 / 16u)];
  float scalar_f32 = uintBitsToFloat(v_36[((v_35 & 15u) >> 2u)]);
  uint v_37 = (4u + (idx * 544u));
  uvec4 v_38 = v.inner[(v_37 / 16u)];
  int scalar_i32 = int(v_38[((v_37 & 15u) >> 2u)]);
  uint v_39 = (8u + (idx * 544u));
  uvec4 v_40 = v.inner[(v_39 / 16u)];
  uint scalar_u32 = v_40[((v_39 & 15u) >> 2u)];
  uint v_41 = (16u + (idx * 544u));
  uvec4 v_42 = v.inner[(v_41 / 16u)];
  vec2 vec2_f32 = uintBitsToFloat(mix(v_42.xy, v_42.zw, bvec2((((v_41 & 15u) >> 2u) == 2u))));
  uint v_43 = (24u + (idx * 544u));
  uvec4 v_44 = v.inner[(v_43 / 16u)];
  ivec2 vec2_i32 = ivec2(mix(v_44.xy, v_44.zw, bvec2((((v_43 & 15u) >> 2u) == 2u))));
  uint v_45 = (32u + (idx * 544u));
  uvec4 v_46 = v.inner[(v_45 / 16u)];
  uvec2 vec2_u32 = mix(v_46.xy, v_46.zw, bvec2((((v_45 & 15u) >> 2u) == 2u)));
  vec3 vec3_f32 = uintBitsToFloat(v.inner[((48u + (idx * 544u)) / 16u)].xyz);
  ivec3 vec3_i32 = ivec3(v.inner[((64u + (idx * 544u)) / 16u)].xyz);
  uvec3 vec3_u32 = v.inner[((80u + (idx * 544u)) / 16u)].xyz;
  vec4 vec4_f32 = uintBitsToFloat(v.inner[((96u + (idx * 544u)) / 16u)]);
  ivec4 vec4_i32 = ivec4(v.inner[((112u + (idx * 544u)) / 16u)]);
  uvec4 vec4_u32 = v.inner[((128u + (idx * 544u)) / 16u)];
  mat2 mat2x2_f32 = v_30((144u + (idx * 544u)));
  mat2x3 mat2x3_f32 = v_29((160u + (idx * 544u)));
  mat2x4 mat2x4_f32 = v_28((192u + (idx * 544u)));
  mat3x2 mat3x2_f32 = v_20((224u + (idx * 544u)));
  mat3 mat3x3_f32 = v_19((256u + (idx * 544u)));
  mat3x4 mat3x4_f32 = v_18((304u + (idx * 544u)));
  mat4x2 mat4x2_f32 = v_7((352u + (idx * 544u)));
  mat4x3 mat4x3_f32 = v_6((384u + (idx * 544u)));
  mat4 mat4x4_f32 = v_5((448u + (idx * 544u)));
  vec3 arr2_vec3_f32[2] = v_2((512u + (idx * 544u)));
  uint v_47 = uint(tint_f32_to_i32(scalar_f32));
  int v_48 = int((v_47 + uint(scalar_i32)));
  int v_49 = int(scalar_u32);
  uint v_50 = uint(v_48);
  int v_51 = int((v_50 + uint(v_49)));
  int v_52 = tint_f32_to_i32(vec2_f32.x);
  uint v_53 = uint(v_51);
  uint v_54 = uint(int((v_53 + uint(v_52))));
  int v_55 = int((v_54 + uint(vec2_i32.x)));
  int v_56 = int(vec2_u32.x);
  uint v_57 = uint(v_55);
  int v_58 = int((v_57 + uint(v_56)));
  int v_59 = tint_f32_to_i32(vec3_f32.y);
  uint v_60 = uint(v_58);
  uint v_61 = uint(int((v_60 + uint(v_59))));
  int v_62 = int((v_61 + uint(vec3_i32.y)));
  int v_63 = int(vec3_u32.y);
  uint v_64 = uint(v_62);
  int v_65 = int((v_64 + uint(v_63)));
  int v_66 = tint_f32_to_i32(vec4_f32.z);
  uint v_67 = uint(v_65);
  uint v_68 = uint(int((v_67 + uint(v_66))));
  int v_69 = int((v_68 + uint(vec4_i32.z)));
  int v_70 = int(vec4_u32.z);
  uint v_71 = uint(v_69);
  int v_72 = int((v_71 + uint(v_70)));
  int v_73 = tint_f32_to_i32(mat2x2_f32[0u].x);
  uint v_74 = uint(v_72);
  int v_75 = int((v_74 + uint(v_73)));
  int v_76 = tint_f32_to_i32(mat2x3_f32[0u].x);
  uint v_77 = uint(v_75);
  int v_78 = int((v_77 + uint(v_76)));
  int v_79 = tint_f32_to_i32(mat2x4_f32[0u].x);
  uint v_80 = uint(v_78);
  int v_81 = int((v_80 + uint(v_79)));
  int v_82 = tint_f32_to_i32(mat3x2_f32[0u].x);
  uint v_83 = uint(v_81);
  int v_84 = int((v_83 + uint(v_82)));
  int v_85 = tint_f32_to_i32(mat3x3_f32[0u].x);
  uint v_86 = uint(v_84);
  int v_87 = int((v_86 + uint(v_85)));
  int v_88 = tint_f32_to_i32(mat3x4_f32[0u].x);
  uint v_89 = uint(v_87);
  int v_90 = int((v_89 + uint(v_88)));
  int v_91 = tint_f32_to_i32(mat4x2_f32[0u].x);
  uint v_92 = uint(v_90);
  int v_93 = int((v_92 + uint(v_91)));
  int v_94 = tint_f32_to_i32(mat4x3_f32[0u].x);
  uint v_95 = uint(v_93);
  int v_96 = int((v_95 + uint(v_94)));
  int v_97 = tint_f32_to_i32(mat4x4_f32[0u].x);
  uint v_98 = uint(v_96);
  int v_99 = int((v_98 + uint(v_97)));
  int v_100 = tint_f32_to_i32(arr2_vec3_f32[0u].x);
  uint v_101 = uint(v_99);
  v_1.inner = int((v_101 + uint(v_100)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
