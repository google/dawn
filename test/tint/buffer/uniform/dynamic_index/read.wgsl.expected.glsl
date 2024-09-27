#version 310 es

int tint_ftoi(float v) {
  return mix(2147483647, mix(int(v), (-2147483647 - 1), (v < -2147483648.0f)), (v <= 2147483520.0f));
}

struct Inner {
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
};

struct Inner_std140 {
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
};

struct S {
  Inner arr[8];
};

struct S_std140 {
  Inner_std140 arr[8];
};

layout(binding = 0, std140) uniform ub_block_std140_ubo {
  S_std140 inner;
} ub;

layout(binding = 1, std430) buffer s_block_ssbo {
  int inner;
} s;

mat2 load_ub_inner_arr_p0_mat2x2_f32(uint p0) {
  uint s_save = p0;
  return mat2(ub.inner.arr[s_save].mat2x2_f32_0, ub.inner.arr[s_save].mat2x2_f32_1);
}

mat3x2 load_ub_inner_arr_p0_mat3x2_f32(uint p0) {
  uint s_save_1 = p0;
  return mat3x2(ub.inner.arr[s_save_1].mat3x2_f32_0, ub.inner.arr[s_save_1].mat3x2_f32_1, ub.inner.arr[s_save_1].mat3x2_f32_2);
}

mat4x2 load_ub_inner_arr_p0_mat4x2_f32(uint p0) {
  uint s_save_2 = p0;
  return mat4x2(ub.inner.arr[s_save_2].mat4x2_f32_0, ub.inner.arr[s_save_2].mat4x2_f32_1, ub.inner.arr[s_save_2].mat4x2_f32_2, ub.inner.arr[s_save_2].mat4x2_f32_3);
}

void tint_symbol(uint idx) {
  float scalar_f32 = ub.inner.arr[idx].scalar_f32;
  int scalar_i32 = ub.inner.arr[idx].scalar_i32;
  uint scalar_u32 = ub.inner.arr[idx].scalar_u32;
  vec2 vec2_f32 = ub.inner.arr[idx].vec2_f32;
  ivec2 vec2_i32 = ub.inner.arr[idx].vec2_i32;
  uvec2 vec2_u32 = ub.inner.arr[idx].vec2_u32;
  vec3 vec3_f32 = ub.inner.arr[idx].vec3_f32;
  ivec3 vec3_i32 = ub.inner.arr[idx].vec3_i32;
  uvec3 vec3_u32 = ub.inner.arr[idx].vec3_u32;
  vec4 vec4_f32 = ub.inner.arr[idx].vec4_f32;
  ivec4 vec4_i32 = ub.inner.arr[idx].vec4_i32;
  uvec4 vec4_u32 = ub.inner.arr[idx].vec4_u32;
  mat2 mat2x2_f32 = load_ub_inner_arr_p0_mat2x2_f32(uint(idx));
  mat2x3 mat2x3_f32 = ub.inner.arr[idx].mat2x3_f32;
  mat2x4 mat2x4_f32 = ub.inner.arr[idx].mat2x4_f32;
  mat3x2 mat3x2_f32 = load_ub_inner_arr_p0_mat3x2_f32(uint(idx));
  mat3 mat3x3_f32 = ub.inner.arr[idx].mat3x3_f32;
  mat3x4 mat3x4_f32 = ub.inner.arr[idx].mat3x4_f32;
  mat4x2 mat4x2_f32 = load_ub_inner_arr_p0_mat4x2_f32(uint(idx));
  mat4x3 mat4x3_f32 = ub.inner.arr[idx].mat4x3_f32;
  mat4 mat4x4_f32 = ub.inner.arr[idx].mat4x4_f32;
  vec3 arr2_vec3_f32[2] = ub.inner.arr[idx].arr2_vec3_f32;
  s.inner = (((((((((((((((((((((tint_ftoi(scalar_f32) + scalar_i32) + int(scalar_u32)) + tint_ftoi(vec2_f32.x)) + vec2_i32.x) + int(vec2_u32.x)) + tint_ftoi(vec3_f32.y)) + vec3_i32.y) + int(vec3_u32.y)) + tint_ftoi(vec4_f32.z)) + vec4_i32.z) + int(vec4_u32.z)) + tint_ftoi(mat2x2_f32[0].x)) + tint_ftoi(mat2x3_f32[0].x)) + tint_ftoi(mat2x4_f32[0].x)) + tint_ftoi(mat3x2_f32[0].x)) + tint_ftoi(mat3x3_f32[0].x)) + tint_ftoi(mat3x4_f32[0].x)) + tint_ftoi(mat4x2_f32[0].x)) + tint_ftoi(mat4x3_f32[0].x)) + tint_ftoi(mat4x4_f32[0].x)) + tint_ftoi(arr2_vec3_f32[0].x));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
