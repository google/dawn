#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};

struct S {
  float scalar_f32;
  int scalar_i32;
  uint scalar_u32;
  float16_t scalar_f16;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  f16vec2 vec2_f16;
  uint pad;
  vec3 vec3_f32;
  uint pad_1;
  ivec3 vec3_i32;
  uint pad_2;
  uvec3 vec3_u32;
  uint pad_3;
  f16vec3 vec3_f16;
  uint pad_4;
  uint pad_5;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  f16vec4 vec4_f16;
  mat2 mat2x2_f32;
  uint pad_6;
  uint pad_7;
  mat2x3 mat2x3_f32;
  mat2x4 mat2x4_f32;
  mat3x2 mat3x2_f32;
  uint pad_8;
  uint pad_9;
  mat3 mat3x3_f32;
  mat3x4 mat3x4_f32;
  mat4x2 mat4x2_f32;
  mat4x3 mat4x3_f32;
  mat4 mat4x4_f32;
  f16mat2 mat2x2_f16;
  f16mat2x3 mat2x3_f16;
  f16mat2x4 mat2x4_f16;
  f16mat3x2 mat3x2_f16;
  uint pad_10;
  f16mat3 mat3x3_f16;
  f16mat3x4 mat3x4_f16;
  f16mat4x2 mat4x2_f16;
  f16mat4x3 mat4x3_f16;
  f16mat4 mat4x4_f16;
  uint pad_11;
  uint pad_12;
  vec3 arr2_vec3_f32[2];
  f16mat4x2 arr2_mat4x2_f16[2];
  Inner struct_inner;
  Inner array_struct_inner[4];
  uint pad_13;
};

layout(binding = 0, std430) buffer sb_block_ssbo {
  S inner;
} sb;

void tint_symbol() {
  float scalar_f32 = sb.inner.scalar_f32;
  int scalar_i32 = sb.inner.scalar_i32;
  uint scalar_u32 = sb.inner.scalar_u32;
  float16_t scalar_f16 = sb.inner.scalar_f16;
  vec2 vec2_f32 = sb.inner.vec2_f32;
  ivec2 vec2_i32 = sb.inner.vec2_i32;
  uvec2 vec2_u32 = sb.inner.vec2_u32;
  f16vec2 vec2_f16 = sb.inner.vec2_f16;
  vec3 vec3_f32 = sb.inner.vec3_f32;
  ivec3 vec3_i32 = sb.inner.vec3_i32;
  uvec3 vec3_u32 = sb.inner.vec3_u32;
  f16vec3 vec3_f16 = sb.inner.vec3_f16;
  vec4 vec4_f32 = sb.inner.vec4_f32;
  ivec4 vec4_i32 = sb.inner.vec4_i32;
  uvec4 vec4_u32 = sb.inner.vec4_u32;
  f16vec4 vec4_f16 = sb.inner.vec4_f16;
  mat2 mat2x2_f32 = sb.inner.mat2x2_f32;
  mat2x3 mat2x3_f32 = sb.inner.mat2x3_f32;
  mat2x4 mat2x4_f32 = sb.inner.mat2x4_f32;
  mat3x2 mat3x2_f32 = sb.inner.mat3x2_f32;
  mat3 mat3x3_f32 = sb.inner.mat3x3_f32;
  mat3x4 mat3x4_f32 = sb.inner.mat3x4_f32;
  mat4x2 mat4x2_f32 = sb.inner.mat4x2_f32;
  mat4x3 mat4x3_f32 = sb.inner.mat4x3_f32;
  mat4 mat4x4_f32 = sb.inner.mat4x4_f32;
  f16mat2 mat2x2_f16 = sb.inner.mat2x2_f16;
  f16mat2x3 mat2x3_f16 = sb.inner.mat2x3_f16;
  f16mat2x4 mat2x4_f16 = sb.inner.mat2x4_f16;
  f16mat3x2 mat3x2_f16 = sb.inner.mat3x2_f16;
  f16mat3 mat3x3_f16 = sb.inner.mat3x3_f16;
  f16mat3x4 mat3x4_f16 = sb.inner.mat3x4_f16;
  f16mat4x2 mat4x2_f16 = sb.inner.mat4x2_f16;
  f16mat4x3 mat4x3_f16 = sb.inner.mat4x3_f16;
  f16mat4 mat4x4_f16 = sb.inner.mat4x4_f16;
  vec3 arr2_vec3_f32[2] = sb.inner.arr2_vec3_f32;
  f16mat4x2 arr2_mat4x2_f16[2] = sb.inner.arr2_mat4x2_f16;
  Inner struct_inner = sb.inner.struct_inner;
  Inner array_struct_inner[4] = sb.inner.array_struct_inner;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
