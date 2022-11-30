#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat4x2_f16_4 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
};

struct Inner {
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
};

struct Inner_std140 {
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
  vec2 mat2x2_f32_0;
  vec2 mat2x2_f32_1;
  uint pad_6;
  uint pad_7;
  mat2x3 mat2x3_f32;
  mat2x4 mat2x4_f32;
  vec2 mat3x2_f32_0;
  vec2 mat3x2_f32_1;
  vec2 mat3x2_f32_2;
  uint pad_8;
  uint pad_9;
  mat3 mat3x3_f32;
  mat3x4 mat3x4_f32;
  vec2 mat4x2_f32_0;
  vec2 mat4x2_f32_1;
  vec2 mat4x2_f32_2;
  vec2 mat4x2_f32_3;
  mat4x3 mat4x3_f32;
  mat4 mat4x4_f32;
  f16vec2 mat2x2_f16_0;
  f16vec2 mat2x2_f16_1;
  f16vec3 mat2x3_f16_0;
  f16vec3 mat2x3_f16_1;
  f16vec4 mat2x4_f16_0;
  f16vec4 mat2x4_f16_1;
  f16vec2 mat3x2_f16_0;
  f16vec2 mat3x2_f16_1;
  f16vec2 mat3x2_f16_2;
  uint pad_10;
  f16vec3 mat3x3_f16_0;
  f16vec3 mat3x3_f16_1;
  f16vec3 mat3x3_f16_2;
  f16vec4 mat3x4_f16_0;
  f16vec4 mat3x4_f16_1;
  f16vec4 mat3x4_f16_2;
  f16vec2 mat4x2_f16_0;
  f16vec2 mat4x2_f16_1;
  f16vec2 mat4x2_f16_2;
  f16vec2 mat4x2_f16_3;
  f16vec3 mat4x3_f16_0;
  f16vec3 mat4x3_f16_1;
  f16vec3 mat4x3_f16_2;
  f16vec3 mat4x3_f16_3;
  f16vec4 mat4x4_f16_0;
  f16vec4 mat4x4_f16_1;
  f16vec4 mat4x4_f16_2;
  f16vec4 mat4x4_f16_3;
  uint pad_11;
  uint pad_12;
  vec3 arr2_vec3_f32[2];
  mat4x2_f16_4 arr2_mat4x2_f16[2];
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

f16mat2 load_ub_inner_arr_p0_mat2x2_f16(uint p0) {
  uint s_save_3 = p0;
  return f16mat2(ub.inner.arr[s_save_3].mat2x2_f16_0, ub.inner.arr[s_save_3].mat2x2_f16_1);
}

f16mat2x3 load_ub_inner_arr_p0_mat2x3_f16(uint p0) {
  uint s_save_4 = p0;
  return f16mat2x3(ub.inner.arr[s_save_4].mat2x3_f16_0, ub.inner.arr[s_save_4].mat2x3_f16_1);
}

f16mat2x4 load_ub_inner_arr_p0_mat2x4_f16(uint p0) {
  uint s_save_5 = p0;
  return f16mat2x4(ub.inner.arr[s_save_5].mat2x4_f16_0, ub.inner.arr[s_save_5].mat2x4_f16_1);
}

f16mat3x2 load_ub_inner_arr_p0_mat3x2_f16(uint p0) {
  uint s_save_6 = p0;
  return f16mat3x2(ub.inner.arr[s_save_6].mat3x2_f16_0, ub.inner.arr[s_save_6].mat3x2_f16_1, ub.inner.arr[s_save_6].mat3x2_f16_2);
}

f16mat3 load_ub_inner_arr_p0_mat3x3_f16(uint p0) {
  uint s_save_7 = p0;
  return f16mat3(ub.inner.arr[s_save_7].mat3x3_f16_0, ub.inner.arr[s_save_7].mat3x3_f16_1, ub.inner.arr[s_save_7].mat3x3_f16_2);
}

f16mat3x4 load_ub_inner_arr_p0_mat3x4_f16(uint p0) {
  uint s_save_8 = p0;
  return f16mat3x4(ub.inner.arr[s_save_8].mat3x4_f16_0, ub.inner.arr[s_save_8].mat3x4_f16_1, ub.inner.arr[s_save_8].mat3x4_f16_2);
}

f16mat4x2 load_ub_inner_arr_p0_mat4x2_f16(uint p0) {
  uint s_save_9 = p0;
  return f16mat4x2(ub.inner.arr[s_save_9].mat4x2_f16_0, ub.inner.arr[s_save_9].mat4x2_f16_1, ub.inner.arr[s_save_9].mat4x2_f16_2, ub.inner.arr[s_save_9].mat4x2_f16_3);
}

f16mat4x3 load_ub_inner_arr_p0_mat4x3_f16(uint p0) {
  uint s_save_10 = p0;
  return f16mat4x3(ub.inner.arr[s_save_10].mat4x3_f16_0, ub.inner.arr[s_save_10].mat4x3_f16_1, ub.inner.arr[s_save_10].mat4x3_f16_2, ub.inner.arr[s_save_10].mat4x3_f16_3);
}

f16mat4 load_ub_inner_arr_p0_mat4x4_f16(uint p0) {
  uint s_save_11 = p0;
  return f16mat4(ub.inner.arr[s_save_11].mat4x4_f16_0, ub.inner.arr[s_save_11].mat4x4_f16_1, ub.inner.arr[s_save_11].mat4x4_f16_2, ub.inner.arr[s_save_11].mat4x4_f16_3);
}

f16mat4x2 conv_mat4x2_f16(mat4x2_f16_4 val) {
  return f16mat4x2(val.col0, val.col1, val.col2, val.col3);
}

f16mat4x2[2] conv_arr2_mat4x2_f16(mat4x2_f16_4 val[2]) {
  f16mat4x2 arr[2] = f16mat4x2[2](f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr[i] = conv_mat4x2_f16(val[i]);
    }
  }
  return arr;
}

void tint_symbol(uint idx) {
  float scalar_f32 = ub.inner.arr[idx].scalar_f32;
  int scalar_i32 = ub.inner.arr[idx].scalar_i32;
  uint scalar_u32 = ub.inner.arr[idx].scalar_u32;
  float16_t scalar_f16 = ub.inner.arr[idx].scalar_f16;
  vec2 vec2_f32 = ub.inner.arr[idx].vec2_f32;
  ivec2 vec2_i32 = ub.inner.arr[idx].vec2_i32;
  uvec2 vec2_u32 = ub.inner.arr[idx].vec2_u32;
  f16vec2 vec2_f16 = ub.inner.arr[idx].vec2_f16;
  vec3 vec3_f32 = ub.inner.arr[idx].vec3_f32;
  ivec3 vec3_i32 = ub.inner.arr[idx].vec3_i32;
  uvec3 vec3_u32 = ub.inner.arr[idx].vec3_u32;
  f16vec3 vec3_f16 = ub.inner.arr[idx].vec3_f16;
  vec4 vec4_f32 = ub.inner.arr[idx].vec4_f32;
  ivec4 vec4_i32 = ub.inner.arr[idx].vec4_i32;
  uvec4 vec4_u32 = ub.inner.arr[idx].vec4_u32;
  f16vec4 vec4_f16 = ub.inner.arr[idx].vec4_f16;
  mat2 mat2x2_f32 = load_ub_inner_arr_p0_mat2x2_f32(uint(idx));
  mat2x3 mat2x3_f32 = ub.inner.arr[idx].mat2x3_f32;
  mat2x4 mat2x4_f32 = ub.inner.arr[idx].mat2x4_f32;
  mat3x2 mat3x2_f32 = load_ub_inner_arr_p0_mat3x2_f32(uint(idx));
  mat3 mat3x3_f32 = ub.inner.arr[idx].mat3x3_f32;
  mat3x4 mat3x4_f32 = ub.inner.arr[idx].mat3x4_f32;
  mat4x2 mat4x2_f32 = load_ub_inner_arr_p0_mat4x2_f32(uint(idx));
  mat4x3 mat4x3_f32 = ub.inner.arr[idx].mat4x3_f32;
  mat4 mat4x4_f32 = ub.inner.arr[idx].mat4x4_f32;
  f16mat2 mat2x2_f16 = load_ub_inner_arr_p0_mat2x2_f16(uint(idx));
  f16mat2x3 mat2x3_f16 = load_ub_inner_arr_p0_mat2x3_f16(uint(idx));
  f16mat2x4 mat2x4_f16 = load_ub_inner_arr_p0_mat2x4_f16(uint(idx));
  f16mat3x2 mat3x2_f16 = load_ub_inner_arr_p0_mat3x2_f16(uint(idx));
  f16mat3 mat3x3_f16 = load_ub_inner_arr_p0_mat3x3_f16(uint(idx));
  f16mat3x4 mat3x4_f16 = load_ub_inner_arr_p0_mat3x4_f16(uint(idx));
  f16mat4x2 mat4x2_f16 = load_ub_inner_arr_p0_mat4x2_f16(uint(idx));
  f16mat4x3 mat4x3_f16 = load_ub_inner_arr_p0_mat4x3_f16(uint(idx));
  f16mat4 mat4x4_f16 = load_ub_inner_arr_p0_mat4x4_f16(uint(idx));
  vec3 arr2_vec3_f32[2] = ub.inner.arr[idx].arr2_vec3_f32;
  f16mat4x2 arr2_mat4x2_f16[2] = conv_arr2_mat4x2_f16(ub.inner.arr[idx].arr2_mat4x2_f16);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
