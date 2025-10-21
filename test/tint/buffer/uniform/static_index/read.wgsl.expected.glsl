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
  int v_4 = int(v_3[((start_byte_offset % 16u) / 4u)]);
  uvec4 v_5 = v.inner[((16u + start_byte_offset) / 16u)];
  return Inner(v_4, uintBitsToFloat(v_5[(((16u + start_byte_offset) % 16u) / 4u)]));
}
Inner[4] v_6(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(0, 0.0f), Inner(0, 0.0f), Inner(0, 0.0f), Inner(0, 0.0f));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_2((start_byte_offset + (v_8 * 32u)));
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
mat4 v_12(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_13(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_14(uint start_byte_offset) {
  uvec4 v_15 = v.inner[(start_byte_offset / 16u)];
  vec2 v_16 = uintBitsToFloat(mix(v_15.xy, v_15.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_17 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_18 = uintBitsToFloat(mix(v_17.xy, v_17.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_19 = v.inner[((16u + start_byte_offset) / 16u)];
  vec2 v_20 = uintBitsToFloat(mix(v_19.xy, v_19.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_21 = v.inner[((24u + start_byte_offset) / 16u)];
  return mat4x2(v_16, v_18, v_20, uintBitsToFloat(mix(v_21.xy, v_21.zw, bvec2(((((24u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat3x4 v_22(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_23(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_24(uint start_byte_offset) {
  uvec4 v_25 = v.inner[(start_byte_offset / 16u)];
  vec2 v_26 = uintBitsToFloat(mix(v_25.xy, v_25.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_27 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_28 = uintBitsToFloat(mix(v_27.xy, v_27.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_29 = v.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_26, v_28, uintBitsToFloat(mix(v_29.xy, v_29.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat2x4 v_30(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_31(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_32(uint start_byte_offset) {
  uvec4 v_33 = v.inner[(start_byte_offset / 16u)];
  vec2 v_34 = uintBitsToFloat(mix(v_33.xy, v_33.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_35 = v.inner[((8u + start_byte_offset) / 16u)];
  return mat2(v_34, uintBitsToFloat(mix(v_35.xy, v_35.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_36 = v.inner[0u];
  float scalar_f32 = uintBitsToFloat(v_36.x);
  uvec4 v_37 = v.inner[0u];
  int scalar_i32 = int(v_37.y);
  uvec4 v_38 = v.inner[0u];
  uint scalar_u32 = v_38.z;
  vec2 vec2_f32 = uintBitsToFloat(v.inner[1u].xy);
  ivec2 vec2_i32 = ivec2(v.inner[1u].zw);
  uvec2 vec2_u32 = v.inner[2u].xy;
  vec3 vec3_f32 = uintBitsToFloat(v.inner[3u].xyz);
  ivec3 vec3_i32 = ivec3(v.inner[4u].xyz);
  uvec3 vec3_u32 = v.inner[5u].xyz;
  vec4 vec4_f32 = uintBitsToFloat(v.inner[6u]);
  ivec4 vec4_i32 = ivec4(v.inner[7u]);
  uvec4 vec4_u32 = v.inner[8u];
  mat2 mat2x2_f32 = v_32(144u);
  mat2x3 mat2x3_f32 = v_31(160u);
  mat2x4 mat2x4_f32 = v_30(192u);
  mat3x2 mat3x2_f32 = v_24(224u);
  mat3 mat3x3_f32 = v_23(256u);
  mat3x4 mat3x4_f32 = v_22(304u);
  mat4x2 mat4x2_f32 = v_14(352u);
  mat4x3 mat4x3_f32 = v_13(384u);
  mat4 mat4x4_f32 = v_12(448u);
  vec3 arr2_vec3_f32[2] = v_9(512u);
  Inner struct_inner = v_2(544u);
  Inner array_struct_inner[4] = v_6(576u);
  uint v_39 = uint(tint_f32_to_i32(scalar_f32));
  int v_40 = int((v_39 + uint(scalar_i32)));
  int v_41 = int(scalar_u32);
  uint v_42 = uint(v_40);
  int v_43 = int((v_42 + uint(v_41)));
  int v_44 = tint_f32_to_i32(vec2_f32.x);
  uint v_45 = uint(v_43);
  uint v_46 = uint(int((v_45 + uint(v_44))));
  int v_47 = int((v_46 + uint(vec2_i32.x)));
  int v_48 = int(vec2_u32.x);
  uint v_49 = uint(v_47);
  int v_50 = int((v_49 + uint(v_48)));
  int v_51 = tint_f32_to_i32(vec3_f32.y);
  uint v_52 = uint(v_50);
  uint v_53 = uint(int((v_52 + uint(v_51))));
  int v_54 = int((v_53 + uint(vec3_i32.y)));
  int v_55 = int(vec3_u32.y);
  uint v_56 = uint(v_54);
  int v_57 = int((v_56 + uint(v_55)));
  int v_58 = tint_f32_to_i32(vec4_f32.z);
  uint v_59 = uint(v_57);
  uint v_60 = uint(int((v_59 + uint(v_58))));
  int v_61 = int((v_60 + uint(vec4_i32.z)));
  int v_62 = int(vec4_u32.z);
  uint v_63 = uint(v_61);
  int v_64 = int((v_63 + uint(v_62)));
  int v_65 = tint_f32_to_i32(mat2x2_f32[0u].x);
  uint v_66 = uint(v_64);
  int v_67 = int((v_66 + uint(v_65)));
  int v_68 = tint_f32_to_i32(mat2x3_f32[0u].x);
  uint v_69 = uint(v_67);
  int v_70 = int((v_69 + uint(v_68)));
  int v_71 = tint_f32_to_i32(mat2x4_f32[0u].x);
  uint v_72 = uint(v_70);
  int v_73 = int((v_72 + uint(v_71)));
  int v_74 = tint_f32_to_i32(mat3x2_f32[0u].x);
  uint v_75 = uint(v_73);
  int v_76 = int((v_75 + uint(v_74)));
  int v_77 = tint_f32_to_i32(mat3x3_f32[0u].x);
  uint v_78 = uint(v_76);
  int v_79 = int((v_78 + uint(v_77)));
  int v_80 = tint_f32_to_i32(mat3x4_f32[0u].x);
  uint v_81 = uint(v_79);
  int v_82 = int((v_81 + uint(v_80)));
  int v_83 = tint_f32_to_i32(mat4x2_f32[0u].x);
  uint v_84 = uint(v_82);
  int v_85 = int((v_84 + uint(v_83)));
  int v_86 = tint_f32_to_i32(mat4x3_f32[0u].x);
  uint v_87 = uint(v_85);
  int v_88 = int((v_87 + uint(v_86)));
  int v_89 = tint_f32_to_i32(mat4x4_f32[0u].x);
  uint v_90 = uint(v_88);
  int v_91 = int((v_90 + uint(v_89)));
  int v_92 = tint_f32_to_i32(arr2_vec3_f32[0u].x);
  uint v_93 = uint(v_91);
  uint v_94 = uint(int((v_93 + uint(v_92))));
  uint v_95 = uint(int((v_94 + uint(struct_inner.scalar_i32))));
  v_1.inner = int((v_95 + uint(array_struct_inner[0u].scalar_i32)));
}
