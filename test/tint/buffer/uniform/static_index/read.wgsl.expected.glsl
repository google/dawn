#version 310 es

int tint_ftoi(float v) {
  return mix(2147483647, mix(int(v), (-2147483647 - 1), (v < -2147483648.0f)), (v <= 2147483520.0f));
}

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
  uint pad;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  uint pad_1;
  uint pad_2;
  vec3 vec3_f32;
  uint pad_3;
  ivec3 vec3_i32;
  uint pad_4;
  uvec3 vec3_u32;
  uint pad_5;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  mat2 mat2x2_f32;
  mat2x3 mat2x3_f32;
  mat2x4 mat2x4_f32;
  mat3x2 mat3x2_f32;
  uint pad_6;
  uint pad_7;
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
  uint pad;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  uint pad_1;
  uint pad_2;
  vec3 vec3_f32;
  uint pad_3;
  ivec3 vec3_i32;
  uint pad_4;
  uvec3 vec3_u32;
  uint pad_5;
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
  uint pad_6;
  uint pad_7;
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

layout(binding = 1, std430) buffer s_block_ssbo {
  int inner;
} s;

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
  s.inner = (((((((((((((((((((((((tint_ftoi(scalar_f32) + scalar_i32) + int(scalar_u32)) + tint_ftoi(vec2_f32.x)) + vec2_i32.x) + int(vec2_u32.x)) + tint_ftoi(vec3_f32.y)) + vec3_i32.y) + int(vec3_u32.y)) + tint_ftoi(vec4_f32.z)) + vec4_i32.z) + int(vec4_u32.z)) + tint_ftoi(mat2x2_f32[0].x)) + tint_ftoi(mat2x3_f32[0].x)) + tint_ftoi(mat2x4_f32[0].x)) + tint_ftoi(mat3x2_f32[0].x)) + tint_ftoi(mat3x3_f32[0].x)) + tint_ftoi(mat3x4_f32[0].x)) + tint_ftoi(mat4x2_f32[0].x)) + tint_ftoi(mat4x3_f32[0].x)) + tint_ftoi(mat4x4_f32[0].x)) + tint_ftoi(arr2_vec3_f32[0].x)) + struct_inner.scalar_i32) + array_struct_inner[0].scalar_i32);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
