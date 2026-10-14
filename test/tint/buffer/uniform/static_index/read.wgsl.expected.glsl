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
    uint idx = 0u;
    while(true) {
      uint v_8 = idx;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_2((start_byte_offset + (v_8 * 32u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
vec3[2] v_9(uint start_byte_offset) {
  vec3 a[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  {
    uint idx = 0u;
    while(true) {
      uint v_10 = idx;
      if ((v_10 >= 2u)) {
        break;
      }
      a[v_10] = uintBitsToFloat(v.inner[((start_byte_offset + (v_10 * 16u)) / 16u)].xyz);
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
mat4 v_11(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_12(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_13(uint start_byte_offset) {
  uvec4 v_14 = v.inner[(start_byte_offset / 16u)];
  vec2 v_15 = uintBitsToFloat(mix(v_14.xy, v_14.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_16 = (8u + start_byte_offset);
  uvec4 v_17 = v.inner[(v_16 / 16u)];
  vec2 v_18 = uintBitsToFloat(mix(v_17.xy, v_17.zw, bvec2((((v_16 & 15u) >> 2u) == 2u))));
  uint v_19 = (16u + start_byte_offset);
  uvec4 v_20 = v.inner[(v_19 / 16u)];
  vec2 v_21 = uintBitsToFloat(mix(v_20.xy, v_20.zw, bvec2((((v_19 & 15u) >> 2u) == 2u))));
  uint v_22 = (24u + start_byte_offset);
  uvec4 v_23 = v.inner[(v_22 / 16u)];
  return mat4x2(v_15, v_18, v_21, uintBitsToFloat(mix(v_23.xy, v_23.zw, bvec2((((v_22 & 15u) >> 2u) == 2u)))));
}
mat3x4 v_24(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_25(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_26(uint start_byte_offset) {
  uvec4 v_27 = v.inner[(start_byte_offset / 16u)];
  vec2 v_28 = uintBitsToFloat(mix(v_27.xy, v_27.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_29 = (8u + start_byte_offset);
  uvec4 v_30 = v.inner[(v_29 / 16u)];
  vec2 v_31 = uintBitsToFloat(mix(v_30.xy, v_30.zw, bvec2((((v_29 & 15u) >> 2u) == 2u))));
  uint v_32 = (16u + start_byte_offset);
  uvec4 v_33 = v.inner[(v_32 / 16u)];
  return mat3x2(v_28, v_31, uintBitsToFloat(mix(v_33.xy, v_33.zw, bvec2((((v_32 & 15u) >> 2u) == 2u)))));
}
mat2x4 v_34(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_35(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_36(uint start_byte_offset) {
  uvec4 v_37 = v.inner[(start_byte_offset / 16u)];
  vec2 v_38 = uintBitsToFloat(mix(v_37.xy, v_37.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_39 = (8u + start_byte_offset);
  uvec4 v_40 = v.inner[(v_39 / 16u)];
  return mat2(v_38, uintBitsToFloat(mix(v_40.xy, v_40.zw, bvec2((((v_39 & 15u) >> 2u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_41 = v.inner[0u];
  float scalar_f32 = uintBitsToFloat(v_41.x);
  uvec4 v_42 = v.inner[0u];
  int scalar_i32 = int(v_42.y);
  uvec4 v_43 = v.inner[0u];
  uint scalar_u32 = v_43.z;
  vec2 vec2_f32 = uintBitsToFloat(v.inner[1u].xy);
  ivec2 vec2_i32 = ivec2(v.inner[1u].zw);
  uvec2 vec2_u32 = v.inner[2u].xy;
  vec3 vec3_f32 = uintBitsToFloat(v.inner[3u].xyz);
  ivec3 vec3_i32 = ivec3(v.inner[4u].xyz);
  uvec3 vec3_u32 = v.inner[5u].xyz;
  vec4 vec4_f32 = uintBitsToFloat(v.inner[6u]);
  ivec4 vec4_i32 = ivec4(v.inner[7u]);
  uvec4 vec4_u32 = v.inner[8u];
  mat2 mat2x2_f32 = v_36(144u);
  mat2x3 mat2x3_f32 = v_35(160u);
  mat2x4 mat2x4_f32 = v_34(192u);
  mat3x2 mat3x2_f32 = v_26(224u);
  mat3 mat3x3_f32 = v_25(256u);
  mat3x4 mat3x4_f32 = v_24(304u);
  mat4x2 mat4x2_f32 = v_13(352u);
  mat4x3 mat4x3_f32 = v_12(384u);
  mat4 mat4x4_f32 = v_11(448u);
  vec3 arr2_vec3_f32[2] = v_9(512u);
  Inner struct_inner = v_2(544u);
  Inner array_struct_inner[4] = v_7(576u);
  uint v_44 = uint(tint_f32_to_i32(scalar_f32));
  int v_45 = int((v_44 + uint(scalar_i32)));
  int v_46 = int(scalar_u32);
  uint v_47 = uint(v_45);
  int v_48 = int((v_47 + uint(v_46)));
  int v_49 = tint_f32_to_i32(vec2_f32.x);
  uint v_50 = uint(v_48);
  uint v_51 = uint(int((v_50 + uint(v_49))));
  int v_52 = int((v_51 + uint(vec2_i32.x)));
  int v_53 = int(vec2_u32.x);
  uint v_54 = uint(v_52);
  int v_55 = int((v_54 + uint(v_53)));
  int v_56 = tint_f32_to_i32(vec3_f32.y);
  uint v_57 = uint(v_55);
  uint v_58 = uint(int((v_57 + uint(v_56))));
  int v_59 = int((v_58 + uint(vec3_i32.y)));
  int v_60 = int(vec3_u32.y);
  uint v_61 = uint(v_59);
  int v_62 = int((v_61 + uint(v_60)));
  int v_63 = tint_f32_to_i32(vec4_f32.z);
  uint v_64 = uint(v_62);
  uint v_65 = uint(int((v_64 + uint(v_63))));
  int v_66 = int((v_65 + uint(vec4_i32.z)));
  int v_67 = int(vec4_u32.z);
  uint v_68 = uint(v_66);
  int v_69 = int((v_68 + uint(v_67)));
  int v_70 = tint_f32_to_i32(mat2x2_f32[0].x);
  uint v_71 = uint(v_69);
  int v_72 = int((v_71 + uint(v_70)));
  int v_73 = tint_f32_to_i32(mat2x3_f32[0].x);
  uint v_74 = uint(v_72);
  int v_75 = int((v_74 + uint(v_73)));
  int v_76 = tint_f32_to_i32(mat2x4_f32[0].x);
  uint v_77 = uint(v_75);
  int v_78 = int((v_77 + uint(v_76)));
  int v_79 = tint_f32_to_i32(mat3x2_f32[0].x);
  uint v_80 = uint(v_78);
  int v_81 = int((v_80 + uint(v_79)));
  int v_82 = tint_f32_to_i32(mat3x3_f32[0].x);
  uint v_83 = uint(v_81);
  int v_84 = int((v_83 + uint(v_82)));
  int v_85 = tint_f32_to_i32(mat3x4_f32[0].x);
  uint v_86 = uint(v_84);
  int v_87 = int((v_86 + uint(v_85)));
  int v_88 = tint_f32_to_i32(mat4x2_f32[0].x);
  uint v_89 = uint(v_87);
  int v_90 = int((v_89 + uint(v_88)));
  int v_91 = tint_f32_to_i32(mat4x3_f32[0].x);
  uint v_92 = uint(v_90);
  int v_93 = int((v_92 + uint(v_91)));
  int v_94 = tint_f32_to_i32(mat4x4_f32[0].x);
  uint v_95 = uint(v_93);
  int v_96 = int((v_95 + uint(v_94)));
  int v_97 = tint_f32_to_i32(arr2_vec3_f32[0].x);
  uint v_98 = uint(v_96);
  uint v_99 = uint(int((v_98 + uint(v_97))));
  uint v_100 = uint(int((v_99 + uint(struct_inner.scalar_i32))));
  v_1.inner = int((v_100 + uint(array_struct_inner[0].scalar_i32)));
}
