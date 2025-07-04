#version 310 es


struct Inner {
  int scalar_i32;
  float scalar_f32;
};

struct S {
  float scalar_f32;
  int scalar_i32;
  uint scalar_u32;
  uint tint_pad_0;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  uint tint_pad_1;
  uint tint_pad_2;
  vec3 vec3_f32;
  uint tint_pad_3;
  ivec3 vec3_i32;
  uint tint_pad_4;
  uvec3 vec3_u32;
  uint tint_pad_5;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  mat2 mat2x2_f32;
  mat2x3 mat2x3_f32;
  mat2x4 mat2x4_f32;
  mat3x2 mat3x2_f32;
  uint tint_pad_6;
  uint tint_pad_7;
  mat3 mat3x3_f32;
  mat3x4 mat3x4_f32;
  mat4x2 mat4x2_f32;
  mat4x3 mat4x3_f32;
  mat4 mat4x4_f32;
  vec3 arr2_vec3_f32[2];
  Inner struct_inner;
  Inner array_struct_inner[4];
  uint tint_pad_8;
  uint tint_pad_9;
};

layout(binding = 0, std430)
buffer sb_block_1_ssbo {
  S inner;
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  int inner;
} v_1;
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float scalar_f32 = v.inner.scalar_f32;
  int scalar_i32 = v.inner.scalar_i32;
  uint scalar_u32 = v.inner.scalar_u32;
  vec2 vec2_f32 = v.inner.vec2_f32;
  ivec2 vec2_i32 = v.inner.vec2_i32;
  uvec2 vec2_u32 = v.inner.vec2_u32;
  vec3 vec3_f32 = v.inner.vec3_f32;
  ivec3 vec3_i32 = v.inner.vec3_i32;
  uvec3 vec3_u32 = v.inner.vec3_u32;
  vec4 vec4_f32 = v.inner.vec4_f32;
  ivec4 vec4_i32 = v.inner.vec4_i32;
  uvec4 vec4_u32 = v.inner.vec4_u32;
  mat2 mat2x2_f32 = v.inner.mat2x2_f32;
  mat2x3 mat2x3_f32 = v.inner.mat2x3_f32;
  mat2x4 mat2x4_f32 = v.inner.mat2x4_f32;
  mat3x2 mat3x2_f32 = v.inner.mat3x2_f32;
  mat3 mat3x3_f32 = v.inner.mat3x3_f32;
  mat3x4 mat3x4_f32 = v.inner.mat3x4_f32;
  mat4x2 mat4x2_f32 = v.inner.mat4x2_f32;
  mat4x3 mat4x3_f32 = v.inner.mat4x3_f32;
  mat4 mat4x4_f32 = v.inner.mat4x4_f32;
  vec3 arr2_vec3_f32[2] = v.inner.arr2_vec3_f32;
  Inner struct_inner = v.inner.struct_inner;
  Inner array_struct_inner[4] = v.inner.array_struct_inner;
  uint v_2 = uint(tint_f32_to_i32(scalar_f32));
  int v_3 = int((v_2 + uint(scalar_i32)));
  int v_4 = int(scalar_u32);
  uint v_5 = uint(v_3);
  int v_6 = int((v_5 + uint(v_4)));
  int v_7 = tint_f32_to_i32(vec2_f32.x);
  uint v_8 = uint(v_6);
  uint v_9 = uint(int((v_8 + uint(v_7))));
  int v_10 = int((v_9 + uint(vec2_i32.x)));
  int v_11 = int(vec2_u32.x);
  uint v_12 = uint(v_10);
  int v_13 = int((v_12 + uint(v_11)));
  int v_14 = tint_f32_to_i32(vec3_f32.y);
  uint v_15 = uint(v_13);
  uint v_16 = uint(int((v_15 + uint(v_14))));
  int v_17 = int((v_16 + uint(vec3_i32.y)));
  int v_18 = int(vec3_u32.y);
  uint v_19 = uint(v_17);
  int v_20 = int((v_19 + uint(v_18)));
  int v_21 = tint_f32_to_i32(vec4_f32.z);
  uint v_22 = uint(v_20);
  uint v_23 = uint(int((v_22 + uint(v_21))));
  int v_24 = int((v_23 + uint(vec4_i32.z)));
  int v_25 = int(vec4_u32.z);
  uint v_26 = uint(v_24);
  int v_27 = int((v_26 + uint(v_25)));
  int v_28 = tint_f32_to_i32(mat2x2_f32[0u].x);
  uint v_29 = uint(v_27);
  int v_30 = int((v_29 + uint(v_28)));
  int v_31 = tint_f32_to_i32(mat2x3_f32[0u].x);
  uint v_32 = uint(v_30);
  int v_33 = int((v_32 + uint(v_31)));
  int v_34 = tint_f32_to_i32(mat2x4_f32[0u].x);
  uint v_35 = uint(v_33);
  int v_36 = int((v_35 + uint(v_34)));
  int v_37 = tint_f32_to_i32(mat3x2_f32[0u].x);
  uint v_38 = uint(v_36);
  int v_39 = int((v_38 + uint(v_37)));
  int v_40 = tint_f32_to_i32(mat3x3_f32[0u].x);
  uint v_41 = uint(v_39);
  int v_42 = int((v_41 + uint(v_40)));
  int v_43 = tint_f32_to_i32(mat3x4_f32[0u].x);
  uint v_44 = uint(v_42);
  int v_45 = int((v_44 + uint(v_43)));
  int v_46 = tint_f32_to_i32(mat4x2_f32[0u].x);
  uint v_47 = uint(v_45);
  int v_48 = int((v_47 + uint(v_46)));
  int v_49 = tint_f32_to_i32(mat4x3_f32[0u].x);
  uint v_50 = uint(v_48);
  int v_51 = int((v_50 + uint(v_49)));
  int v_52 = tint_f32_to_i32(mat4x4_f32[0u].x);
  uint v_53 = uint(v_51);
  int v_54 = int((v_53 + uint(v_52)));
  int v_55 = tint_f32_to_i32(arr2_vec3_f32[0u].x);
  uint v_56 = uint(v_54);
  uint v_57 = uint(int((v_56 + uint(v_55))));
  uint v_58 = uint(int((v_57 + uint(struct_inner.scalar_i32))));
  v_1.inner = int((v_58 + uint(array_struct_inner[0u].scalar_i32)));
}
