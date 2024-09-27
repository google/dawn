#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat4x2_f16_4 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
};

int tint_ftoi(float v) {
  return mix(2147483647, mix(int(v), (-2147483647 - 1), (v < -2147483648.0f)), (v <= 2147483520.0f));
}

struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
  uint pad;
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
};

struct S_std140 {
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

f16mat2 load_ub_inner_mat2x2_f16() {
  return f16mat2(ub.inner.mat2x2_f16_0, ub.inner.mat2x2_f16_1);
}

f16mat2x3 load_ub_inner_mat2x3_f16() {
  return f16mat2x3(ub.inner.mat2x3_f16_0, ub.inner.mat2x3_f16_1);
}

f16mat2x4 load_ub_inner_mat2x4_f16() {
  return f16mat2x4(ub.inner.mat2x4_f16_0, ub.inner.mat2x4_f16_1);
}

f16mat3x2 load_ub_inner_mat3x2_f16() {
  return f16mat3x2(ub.inner.mat3x2_f16_0, ub.inner.mat3x2_f16_1, ub.inner.mat3x2_f16_2);
}

f16mat3 load_ub_inner_mat3x3_f16() {
  return f16mat3(ub.inner.mat3x3_f16_0, ub.inner.mat3x3_f16_1, ub.inner.mat3x3_f16_2);
}

f16mat3x4 load_ub_inner_mat3x4_f16() {
  return f16mat3x4(ub.inner.mat3x4_f16_0, ub.inner.mat3x4_f16_1, ub.inner.mat3x4_f16_2);
}

f16mat4x2 load_ub_inner_mat4x2_f16() {
  return f16mat4x2(ub.inner.mat4x2_f16_0, ub.inner.mat4x2_f16_1, ub.inner.mat4x2_f16_2, ub.inner.mat4x2_f16_3);
}

f16mat4x3 load_ub_inner_mat4x3_f16() {
  return f16mat4x3(ub.inner.mat4x3_f16_0, ub.inner.mat4x3_f16_1, ub.inner.mat4x3_f16_2, ub.inner.mat4x3_f16_3);
}

f16mat4 load_ub_inner_mat4x4_f16() {
  return f16mat4(ub.inner.mat4x4_f16_0, ub.inner.mat4x4_f16_1, ub.inner.mat4x4_f16_2, ub.inner.mat4x4_f16_3);
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

void tint_symbol() {
  float scalar_f32 = ub.inner.scalar_f32;
  int scalar_i32 = ub.inner.scalar_i32;
  uint scalar_u32 = ub.inner.scalar_u32;
  float16_t scalar_f16 = ub.inner.scalar_f16;
  vec2 vec2_f32 = ub.inner.vec2_f32;
  ivec2 vec2_i32 = ub.inner.vec2_i32;
  uvec2 vec2_u32 = ub.inner.vec2_u32;
  f16vec2 vec2_f16 = ub.inner.vec2_f16;
  vec3 vec3_f32 = ub.inner.vec3_f32;
  ivec3 vec3_i32 = ub.inner.vec3_i32;
  uvec3 vec3_u32 = ub.inner.vec3_u32;
  f16vec3 vec3_f16 = ub.inner.vec3_f16;
  vec4 vec4_f32 = ub.inner.vec4_f32;
  ivec4 vec4_i32 = ub.inner.vec4_i32;
  uvec4 vec4_u32 = ub.inner.vec4_u32;
  f16vec4 vec4_f16 = ub.inner.vec4_f16;
  mat2 mat2x2_f32 = load_ub_inner_mat2x2_f32();
  mat2x3 mat2x3_f32 = ub.inner.mat2x3_f32;
  mat2x4 mat2x4_f32 = ub.inner.mat2x4_f32;
  mat3x2 mat3x2_f32 = load_ub_inner_mat3x2_f32();
  mat3 mat3x3_f32 = ub.inner.mat3x3_f32;
  mat3x4 mat3x4_f32 = ub.inner.mat3x4_f32;
  mat4x2 mat4x2_f32 = load_ub_inner_mat4x2_f32();
  mat4x3 mat4x3_f32 = ub.inner.mat4x3_f32;
  mat4 mat4x4_f32 = ub.inner.mat4x4_f32;
  f16mat2 mat2x2_f16 = load_ub_inner_mat2x2_f16();
  f16mat2x3 mat2x3_f16 = load_ub_inner_mat2x3_f16();
  f16mat2x4 mat2x4_f16 = load_ub_inner_mat2x4_f16();
  f16mat3x2 mat3x2_f16 = load_ub_inner_mat3x2_f16();
  f16mat3 mat3x3_f16 = load_ub_inner_mat3x3_f16();
  f16mat3x4 mat3x4_f16 = load_ub_inner_mat3x4_f16();
  f16mat4x2 mat4x2_f16 = load_ub_inner_mat4x2_f16();
  f16mat4x3 mat4x3_f16 = load_ub_inner_mat4x3_f16();
  f16mat4 mat4x4_f16 = load_ub_inner_mat4x4_f16();
  vec3 arr2_vec3_f32[2] = ub.inner.arr2_vec3_f32;
  f16mat4x2 arr2_mat4x2_f16[2] = conv_arr2_mat4x2_f16(ub.inner.arr2_mat4x2_f16);
  Inner struct_inner = ub.inner.struct_inner;
  Inner array_struct_inner[4] = ub.inner.array_struct_inner;
  s.inner = (((((((((((((((((((((((((((((((((((((tint_ftoi(scalar_f32) + scalar_i32) + int(scalar_u32)) + int(scalar_f16)) + tint_ftoi(vec2_f32.x)) + vec2_i32.x) + int(vec2_u32.x)) + int(vec2_f16.x)) + tint_ftoi(vec3_f32.y)) + vec3_i32.y) + int(vec3_u32.y)) + int(vec3_f16.y)) + tint_ftoi(vec4_f32.z)) + vec4_i32.z) + int(vec4_u32.z)) + int(vec4_f16.z)) + tint_ftoi(mat2x2_f32[0].x)) + tint_ftoi(mat2x3_f32[0].x)) + tint_ftoi(mat2x4_f32[0].x)) + tint_ftoi(mat3x2_f32[0].x)) + tint_ftoi(mat3x3_f32[0].x)) + tint_ftoi(mat3x4_f32[0].x)) + tint_ftoi(mat4x2_f32[0].x)) + tint_ftoi(mat4x3_f32[0].x)) + tint_ftoi(mat4x4_f32[0].x)) + int(mat2x2_f16[0].x)) + int(mat2x3_f16[0].x)) + int(mat2x4_f16[0].x)) + int(mat3x2_f16[0].x)) + int(mat3x3_f16[0].x)) + int(mat3x4_f16[0].x)) + int(mat4x2_f16[0].x)) + int(mat4x3_f16[0].x)) + int(mat4x4_f16[0].x)) + tint_ftoi(arr2_vec3_f32[0].x)) + int(arr2_mat4x2_f16[0][0].x)) + struct_inner.scalar_i32) + array_struct_inner[0].scalar_i32);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
