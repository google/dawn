#version 310 es

struct Inner {
  int scalar_i32;
  uint pad;
  uint pad_1;
  uint pad_2;
  float scalar_f32;
  uint pad_3;
  uint pad_4;
  uint pad_5;
};

struct S {
  float scalar_f32;
  int scalar_i32;
  uint scalar_u32;
  uint pad_6;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  uint pad_7;
  uint pad_8;
  vec3 vec3_f32;
  uint pad_9;
  ivec3 vec3_i32;
  uint pad_10;
  uvec3 vec3_u32;
  uint pad_11;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  mat2 mat2x2_f32;
  mat2x3 mat2x3_f32;
  mat2x4 mat2x4_f32;
  mat3x2 mat3x2_f32;
  uint pad_12;
  uint pad_13;
  mat3 mat3x3_f32;
  mat3x4 mat3x4_f32;
  mat4x2 mat4x2_f32;
  mat4x3 mat4x3_f32;
  mat4 mat4x4_f32;
  vec3 arr2_vec3_f32[2];
  Inner struct_inner;
  Inner array_struct_inner[4];
};

struct S_std140 {
  float scalar_f32;
  int scalar_i32;
  uint scalar_u32;
  uint pad_6;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  uint pad_7;
  uint pad_8;
  vec3 vec3_f32;
  uint pad_9;
  ivec3 vec3_i32;
  uint pad_10;
  uvec3 vec3_u32;
  uint pad_11;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  vec2 mat2x2_f32_0;
  vec2 mat2x2_f32_1;
  mat2x3 mat2x3_f32;
  mat2x4 mat2x4_f32;
  vec2 mat3x2_f32_0;
  vec2 mat3x2_f32_1;
  vec2 mat3x2_f32_2;
  uint pad_12;
  uint pad_13;
  mat3 mat3x3_f32;
  mat3x4 mat3x4_f32;
  vec2 mat4x2_f32_0;
  vec2 mat4x2_f32_1;
  vec2 mat4x2_f32_2;
  vec2 mat4x2_f32_3;
  mat4x3 mat4x3_f32;
  mat4 mat4x4_f32;
  vec3 arr2_vec3_f32[2];
  Inner struct_inner;
  Inner array_struct_inner[4];
};

layout(binding = 0, std140) uniform ub_block_std140_ubo {
  S_std140 inner;
} ub;

mat2 load_ub_inner_mat2x2_f32() {
  return mat2(ub.inner.mat2x2_f32_0, ub.inner.mat2x2_f32_1);
}

mat3x2 load_ub_inner_mat3x2_f32() {
  return mat3x2(ub.inner.mat3x2_f32_0, ub.inner.mat3x2_f32_1, ub.inner.mat3x2_f32_2);
}

mat4x2 load_ub_inner_mat4x2_f32() {
  return mat4x2(ub.inner.mat4x2_f32_0, ub.inner.mat4x2_f32_1, ub.inner.mat4x2_f32_2, ub.inner.mat4x2_f32_3);
}

void tint_symbol() {
  float scalar_f32 = ub.inner.scalar_f32;
  int scalar_i32 = ub.inner.scalar_i32;
  uint scalar_u32 = ub.inner.scalar_u32;
  vec2 vec2_f32 = ub.inner.vec2_f32;
  ivec2 vec2_i32 = ub.inner.vec2_i32;
  uvec2 vec2_u32 = ub.inner.vec2_u32;
  vec3 vec3_f32 = ub.inner.vec3_f32;
  ivec3 vec3_i32 = ub.inner.vec3_i32;
  uvec3 vec3_u32 = ub.inner.vec3_u32;
  vec4 vec4_f32 = ub.inner.vec4_f32;
  ivec4 vec4_i32 = ub.inner.vec4_i32;
  uvec4 vec4_u32 = ub.inner.vec4_u32;
  mat2 mat2x2_f32 = load_ub_inner_mat2x2_f32();
  mat2x3 mat2x3_f32 = ub.inner.mat2x3_f32;
  mat2x4 mat2x4_f32 = ub.inner.mat2x4_f32;
  mat3x2 mat3x2_f32 = load_ub_inner_mat3x2_f32();
  mat3 mat3x3_f32 = ub.inner.mat3x3_f32;
  mat3x4 mat3x4_f32 = ub.inner.mat3x4_f32;
  mat4x2 mat4x2_f32 = load_ub_inner_mat4x2_f32();
  mat4x3 mat4x3_f32 = ub.inner.mat4x3_f32;
  mat4 mat4x4_f32 = ub.inner.mat4x4_f32;
  vec3 arr2_vec3_f32[2] = ub.inner.arr2_vec3_f32;
  Inner struct_inner = ub.inner.struct_inner;
  Inner array_struct_inner[4] = ub.inner.array_struct_inner;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
