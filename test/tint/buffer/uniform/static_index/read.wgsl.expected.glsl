#version 310 es


struct Inner {
  int scalar_i32;
  float scalar_f32;
};

layout(binding = 0, std140)
uniform ub_block_1_ubo {
  uvec4 inner[44];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  int inner;
} v_1;
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
Inner v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  int v_4 = int(v_3[((start_byte_offset & 15u) >> 2u)]);
  uint v_5 = (16u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  return Inner(v_4, uintBitsToFloat(v_6[((v_5 & 15u) >> 2u)]));
}
Inner[4] v_7(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(0, 0.0f), Inner(0, 0.0f), Inner(0, 0.0f), Inner(0, 0.0f));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_2((start_byte_offset + (v_9 * 32u)));
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  return a;
}
vec3[2] v_10(uint start_byte_offset) {
  vec3 a[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 2u)) {
        break;
      }
      a[v_12] = uintBitsToFloat(v.inner[((start_byte_offset + (v_12 * 16u)) / 16u)].xyz);
      {
        v_11 = (v_12 + 1u);
      }
    }
  }
  return a;
}
mat4 v_13(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_14(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_15(uint start_byte_offset) {
  uvec4 v_16 = v.inner[(start_byte_offset / 16u)];
  vec2 v_17 = uintBitsToFloat(mix(v_16.xy, v_16.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_18 = (8u + start_byte_offset);
  uvec4 v_19 = v.inner[(v_18 / 16u)];
  vec2 v_20 = uintBitsToFloat(mix(v_19.xy, v_19.zw, bvec2((((v_18 & 15u) >> 2u) == 2u))));
  uint v_21 = (16u + start_byte_offset);
  uvec4 v_22 = v.inner[(v_21 / 16u)];
  vec2 v_23 = uintBitsToFloat(mix(v_22.xy, v_22.zw, bvec2((((v_21 & 15u) >> 2u) == 2u))));
  uint v_24 = (24u + start_byte_offset);
  uvec4 v_25 = v.inner[(v_24 / 16u)];
  return mat4x2(v_17, v_20, v_23, uintBitsToFloat(mix(v_25.xy, v_25.zw, bvec2((((v_24 & 15u) >> 2u) == 2u)))));
}
mat3x4 v_26(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_27(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_28(uint start_byte_offset) {
  uvec4 v_29 = v.inner[(start_byte_offset / 16u)];
  vec2 v_30 = uintBitsToFloat(mix(v_29.xy, v_29.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_31 = (8u + start_byte_offset);
  uvec4 v_32 = v.inner[(v_31 / 16u)];
  vec2 v_33 = uintBitsToFloat(mix(v_32.xy, v_32.zw, bvec2((((v_31 & 15u) >> 2u) == 2u))));
  uint v_34 = (16u + start_byte_offset);
  uvec4 v_35 = v.inner[(v_34 / 16u)];
  return mat3x2(v_30, v_33, uintBitsToFloat(mix(v_35.xy, v_35.zw, bvec2((((v_34 & 15u) >> 2u) == 2u)))));
}
mat2x4 v_36(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_37(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_38(uint start_byte_offset) {
  uvec4 v_39 = v.inner[(start_byte_offset / 16u)];
  vec2 v_40 = uintBitsToFloat(mix(v_39.xy, v_39.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_41 = (8u + start_byte_offset);
  uvec4 v_42 = v.inner[(v_41 / 16u)];
  return mat2(v_40, uintBitsToFloat(mix(v_42.xy, v_42.zw, bvec2((((v_41 & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_43 = v.inner[0u];
  float scalar_f32 = uintBitsToFloat(v_43.x);
  uvec4 v_44 = v.inner[0u];
  int scalar_i32 = int(v_44.y);
  uvec4 v_45 = v.inner[0u];
  uint scalar_u32 = v_45.z;
  vec2 vec2_f32 = uintBitsToFloat(v.inner[1u].xy);
  ivec2 vec2_i32 = ivec2(v.inner[1u].zw);
  uvec2 vec2_u32 = v.inner[2u].xy;
  vec3 vec3_f32 = uintBitsToFloat(v.inner[3u].xyz);
  ivec3 vec3_i32 = ivec3(v.inner[4u].xyz);
  uvec3 vec3_u32 = v.inner[5u].xyz;
  vec4 vec4_f32 = uintBitsToFloat(v.inner[6u]);
  ivec4 vec4_i32 = ivec4(v.inner[7u]);
  uvec4 vec4_u32 = v.inner[8u];
  mat2 mat2x2_f32 = v_38(144u);
  mat2x3 mat2x3_f32 = v_37(160u);
  mat2x4 mat2x4_f32 = v_36(192u);
  mat3x2 mat3x2_f32 = v_28(224u);
  mat3 mat3x3_f32 = v_27(256u);
  mat3x4 mat3x4_f32 = v_26(304u);
  mat4x2 mat4x2_f32 = v_15(352u);
  mat4x3 mat4x3_f32 = v_14(384u);
  mat4 mat4x4_f32 = v_13(448u);
  vec3 arr2_vec3_f32[2] = v_10(512u);
  Inner struct_inner = v_2(544u);
  Inner array_struct_inner[4] = v_7(576u);
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
  int v_72 = tint_f32_to_i32(mat2x2_f32[0u].x);
  uint v_73 = uint(v_71);
  int v_74 = int((v_73 + uint(v_72)));
  int v_75 = tint_f32_to_i32(mat2x3_f32[0u].x);
  uint v_76 = uint(v_74);
  int v_77 = int((v_76 + uint(v_75)));
  int v_78 = tint_f32_to_i32(mat2x4_f32[0u].x);
  uint v_79 = uint(v_77);
  int v_80 = int((v_79 + uint(v_78)));
  int v_81 = tint_f32_to_i32(mat3x2_f32[0u].x);
  uint v_82 = uint(v_80);
  int v_83 = int((v_82 + uint(v_81)));
  int v_84 = tint_f32_to_i32(mat3x3_f32[0u].x);
  uint v_85 = uint(v_83);
  int v_86 = int((v_85 + uint(v_84)));
  int v_87 = tint_f32_to_i32(mat3x4_f32[0u].x);
  uint v_88 = uint(v_86);
  int v_89 = int((v_88 + uint(v_87)));
  int v_90 = tint_f32_to_i32(mat4x2_f32[0u].x);
  uint v_91 = uint(v_89);
  int v_92 = int((v_91 + uint(v_90)));
  int v_93 = tint_f32_to_i32(mat4x3_f32[0u].x);
  uint v_94 = uint(v_92);
  int v_95 = int((v_94 + uint(v_93)));
  int v_96 = tint_f32_to_i32(mat4x4_f32[0u].x);
  uint v_97 = uint(v_95);
  int v_98 = int((v_97 + uint(v_96)));
  int v_99 = tint_f32_to_i32(arr2_vec3_f32[0u].x);
  uint v_100 = uint(v_98);
  uint v_101 = uint(int((v_100 + uint(v_99))));
  uint v_102 = uint(int((v_101 + uint(struct_inner.scalar_i32))));
  v_1.inner = int((v_102 + uint(array_struct_inner[0u].scalar_i32)));
}
