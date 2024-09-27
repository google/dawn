#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat4x2_f16_std140 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
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
  vec3 vec3_f32;
  ivec3 vec3_i32;
  uvec3 vec3_u32;
  f16vec3 vec3_f16;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  f16vec4 vec4_f16;
  vec2 mat2x2_f32_col0;
  vec2 mat2x2_f32_col1;
  vec3 mat2x3_f32_col0;
  vec3 mat2x3_f32_col1;
  mat2x4 mat2x4_f32;
  vec2 mat3x2_f32_col0;
  vec2 mat3x2_f32_col1;
  vec2 mat3x2_f32_col2;
  vec3 mat3x3_f32_col0;
  vec3 mat3x3_f32_col1;
  vec3 mat3x3_f32_col2;
  mat3x4 mat3x4_f32;
  vec2 mat4x2_f32_col0;
  vec2 mat4x2_f32_col1;
  vec2 mat4x2_f32_col2;
  vec2 mat4x2_f32_col3;
  vec3 mat4x3_f32_col0;
  vec3 mat4x3_f32_col1;
  vec3 mat4x3_f32_col2;
  vec3 mat4x3_f32_col3;
  mat4 mat4x4_f32;
  f16vec2 mat2x2_f16_col0;
  f16vec2 mat2x2_f16_col1;
  f16vec3 mat2x3_f16_col0;
  f16vec3 mat2x3_f16_col1;
  f16vec4 mat2x4_f16_col0;
  f16vec4 mat2x4_f16_col1;
  f16vec2 mat3x2_f16_col0;
  f16vec2 mat3x2_f16_col1;
  f16vec2 mat3x2_f16_col2;
  f16vec3 mat3x3_f16_col0;
  f16vec3 mat3x3_f16_col1;
  f16vec3 mat3x3_f16_col2;
  f16vec4 mat3x4_f16_col0;
  f16vec4 mat3x4_f16_col1;
  f16vec4 mat3x4_f16_col2;
  f16vec2 mat4x2_f16_col0;
  f16vec2 mat4x2_f16_col1;
  f16vec2 mat4x2_f16_col2;
  f16vec2 mat4x2_f16_col3;
  f16vec3 mat4x3_f16_col0;
  f16vec3 mat4x3_f16_col1;
  f16vec3 mat4x3_f16_col2;
  f16vec3 mat4x3_f16_col3;
  f16vec4 mat4x4_f16_col0;
  f16vec4 mat4x4_f16_col1;
  f16vec4 mat4x4_f16_col2;
  f16vec4 mat4x4_f16_col3;
  vec3 arr2_vec3_f32[2];
  mat4x2_f16_std140 arr2_mat4x2_f16[2];
};

struct S_std140 {
  Inner_std140 arr[8];
};

layout(binding = 0, std140)
uniform tint_symbol_2_std140_1_ubo {
  S_std140 tint_symbol_1;
} v;
layout(binding = 1, std430)
buffer tint_symbol_4_1_ssbo {
  int tint_symbol_3;
} v_1;
int tint_f16_to_i32(float16_t value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -65504.0hf)), (value <= 65504.0hf));
}
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
void tint_symbol_inner(uint idx) {
  float scalar_f32 = v.tint_symbol_1.arr[idx].scalar_f32;
  int scalar_i32 = v.tint_symbol_1.arr[idx].scalar_i32;
  uint scalar_u32 = v.tint_symbol_1.arr[idx].scalar_u32;
  float16_t scalar_f16 = v.tint_symbol_1.arr[idx].scalar_f16;
  vec2 vec2_f32 = v.tint_symbol_1.arr[idx].vec2_f32;
  ivec2 vec2_i32 = v.tint_symbol_1.arr[idx].vec2_i32;
  uvec2 vec2_u32 = v.tint_symbol_1.arr[idx].vec2_u32;
  f16vec2 vec2_f16 = v.tint_symbol_1.arr[idx].vec2_f16;
  vec3 vec3_f32 = v.tint_symbol_1.arr[idx].vec3_f32;
  ivec3 vec3_i32 = v.tint_symbol_1.arr[idx].vec3_i32;
  uvec3 vec3_u32 = v.tint_symbol_1.arr[idx].vec3_u32;
  f16vec3 vec3_f16 = v.tint_symbol_1.arr[idx].vec3_f16;
  vec4 vec4_f32 = v.tint_symbol_1.arr[idx].vec4_f32;
  ivec4 vec4_i32 = v.tint_symbol_1.arr[idx].vec4_i32;
  uvec4 vec4_u32 = v.tint_symbol_1.arr[idx].vec4_u32;
  f16vec4 vec4_f16 = v.tint_symbol_1.arr[idx].vec4_f16;
  mat2 mat2x2_f32 = mat2(v.tint_symbol_1.arr[idx].mat2x2_f32_col0, v.tint_symbol_1.arr[idx].mat2x2_f32_col1);
  mat2x3 mat2x3_f32 = mat2x3(v.tint_symbol_1.arr[idx].mat2x3_f32_col0, v.tint_symbol_1.arr[idx].mat2x3_f32_col1);
  mat2x4 mat2x4_f32 = v.tint_symbol_1.arr[idx].mat2x4_f32;
  mat3x2 mat3x2_f32 = mat3x2(v.tint_symbol_1.arr[idx].mat3x2_f32_col0, v.tint_symbol_1.arr[idx].mat3x2_f32_col1, v.tint_symbol_1.arr[idx].mat3x2_f32_col2);
  mat3 mat3x3_f32 = mat3(v.tint_symbol_1.arr[idx].mat3x3_f32_col0, v.tint_symbol_1.arr[idx].mat3x3_f32_col1, v.tint_symbol_1.arr[idx].mat3x3_f32_col2);
  mat3x4 mat3x4_f32 = v.tint_symbol_1.arr[idx].mat3x4_f32;
  mat4x2 mat4x2_f32 = mat4x2(v.tint_symbol_1.arr[idx].mat4x2_f32_col0, v.tint_symbol_1.arr[idx].mat4x2_f32_col1, v.tint_symbol_1.arr[idx].mat4x2_f32_col2, v.tint_symbol_1.arr[idx].mat4x2_f32_col3);
  mat4x3 mat4x3_f32 = mat4x3(v.tint_symbol_1.arr[idx].mat4x3_f32_col0, v.tint_symbol_1.arr[idx].mat4x3_f32_col1, v.tint_symbol_1.arr[idx].mat4x3_f32_col2, v.tint_symbol_1.arr[idx].mat4x3_f32_col3);
  mat4 mat4x4_f32 = v.tint_symbol_1.arr[idx].mat4x4_f32;
  f16mat2 mat2x2_f16 = f16mat2(v.tint_symbol_1.arr[idx].mat2x2_f16_col0, v.tint_symbol_1.arr[idx].mat2x2_f16_col1);
  f16mat2x3 mat2x3_f16 = f16mat2x3(v.tint_symbol_1.arr[idx].mat2x3_f16_col0, v.tint_symbol_1.arr[idx].mat2x3_f16_col1);
  f16mat2x4 mat2x4_f16 = f16mat2x4(v.tint_symbol_1.arr[idx].mat2x4_f16_col0, v.tint_symbol_1.arr[idx].mat2x4_f16_col1);
  f16mat3x2 mat3x2_f16 = f16mat3x2(v.tint_symbol_1.arr[idx].mat3x2_f16_col0, v.tint_symbol_1.arr[idx].mat3x2_f16_col1, v.tint_symbol_1.arr[idx].mat3x2_f16_col2);
  f16mat3 mat3x3_f16 = f16mat3(v.tint_symbol_1.arr[idx].mat3x3_f16_col0, v.tint_symbol_1.arr[idx].mat3x3_f16_col1, v.tint_symbol_1.arr[idx].mat3x3_f16_col2);
  f16mat3x4 mat3x4_f16 = f16mat3x4(v.tint_symbol_1.arr[idx].mat3x4_f16_col0, v.tint_symbol_1.arr[idx].mat3x4_f16_col1, v.tint_symbol_1.arr[idx].mat3x4_f16_col2);
  f16mat4x2 mat4x2_f16 = f16mat4x2(v.tint_symbol_1.arr[idx].mat4x2_f16_col0, v.tint_symbol_1.arr[idx].mat4x2_f16_col1, v.tint_symbol_1.arr[idx].mat4x2_f16_col2, v.tint_symbol_1.arr[idx].mat4x2_f16_col3);
  f16mat4x3 mat4x3_f16 = f16mat4x3(v.tint_symbol_1.arr[idx].mat4x3_f16_col0, v.tint_symbol_1.arr[idx].mat4x3_f16_col1, v.tint_symbol_1.arr[idx].mat4x3_f16_col2, v.tint_symbol_1.arr[idx].mat4x3_f16_col3);
  f16mat4 mat4x4_f16 = f16mat4(v.tint_symbol_1.arr[idx].mat4x4_f16_col0, v.tint_symbol_1.arr[idx].mat4x4_f16_col1, v.tint_symbol_1.arr[idx].mat4x4_f16_col2, v.tint_symbol_1.arr[idx].mat4x4_f16_col3);
  vec3 arr2_vec3_f32[2] = v.tint_symbol_1.arr[idx].arr2_vec3_f32;
  mat4x2_f16_std140 v_2[2] = v.tint_symbol_1.arr[idx].arr2_mat4x2_f16;
  f16mat4x2 v_3[2] = f16mat4x2[2](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 2u)) {
        break;
      }
      v_3[v_5] = f16mat4x2(v_2[v_5].col0, v_2[v_5].col1, v_2[v_5].col2, v_2[v_5].col3);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  f16mat4x2 arr2_mat4x2_f16[2] = v_3;
  int v_6 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_7 = (v_6 + int(scalar_u32));
  int v_8 = (v_7 + tint_f16_to_i32(scalar_f16));
  int v_9 = ((v_8 + tint_f32_to_i32(vec2_f32[0u])) + vec2_i32[0u]);
  int v_10 = (v_9 + int(vec2_u32[0u]));
  int v_11 = (v_10 + tint_f16_to_i32(vec2_f16[0u]));
  int v_12 = ((v_11 + tint_f32_to_i32(vec3_f32[1u])) + vec3_i32[1u]);
  int v_13 = (v_12 + int(vec3_u32[1u]));
  int v_14 = (v_13 + tint_f16_to_i32(vec3_f16[1u]));
  int v_15 = ((v_14 + tint_f32_to_i32(vec4_f32[2u])) + vec4_i32[2u]);
  int v_16 = (v_15 + int(vec4_u32[2u]));
  int v_17 = (v_16 + tint_f16_to_i32(vec4_f16[2u]));
  int v_18 = (v_17 + tint_f32_to_i32(mat2x2_f32[0][0u]));
  int v_19 = (v_18 + tint_f32_to_i32(mat2x3_f32[0][0u]));
  int v_20 = (v_19 + tint_f32_to_i32(mat2x4_f32[0][0u]));
  int v_21 = (v_20 + tint_f32_to_i32(mat3x2_f32[0][0u]));
  int v_22 = (v_21 + tint_f32_to_i32(mat3x3_f32[0][0u]));
  int v_23 = (v_22 + tint_f32_to_i32(mat3x4_f32[0][0u]));
  int v_24 = (v_23 + tint_f32_to_i32(mat4x2_f32[0][0u]));
  int v_25 = (v_24 + tint_f32_to_i32(mat4x3_f32[0][0u]));
  int v_26 = (v_25 + tint_f32_to_i32(mat4x4_f32[0][0u]));
  int v_27 = (v_26 + tint_f16_to_i32(mat2x2_f16[0][0u]));
  int v_28 = (v_27 + tint_f16_to_i32(mat2x3_f16[0][0u]));
  int v_29 = (v_28 + tint_f16_to_i32(mat2x4_f16[0][0u]));
  int v_30 = (v_29 + tint_f16_to_i32(mat3x2_f16[0][0u]));
  int v_31 = (v_30 + tint_f16_to_i32(mat3x3_f16[0][0u]));
  int v_32 = (v_31 + tint_f16_to_i32(mat3x4_f16[0][0u]));
  int v_33 = (v_32 + tint_f16_to_i32(mat4x2_f16[0][0u]));
  int v_34 = (v_33 + tint_f16_to_i32(mat4x3_f16[0][0u]));
  int v_35 = (v_34 + tint_f16_to_i32(mat4x4_f16[0][0u]));
  int v_36 = (v_35 + tint_f32_to_i32(arr2_vec3_f32[0][0u]));
  v_1.tint_symbol_3 = (v_36 + tint_f16_to_i32(arr2_mat4x2_f16[0][0][0u]));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationIndex);
}
