#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

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

layout(binding = 0, std430) buffer S_ssbo {
  Inner arr[];
} sb;

void assign_and_preserve_padding_sb_arr_X_arr2_vec3_f32(uint dest[1], vec3 value[2]) {
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      sb.arr[dest[0]].arr2_vec3_f32[i] = value[i];
    }
  }
}

void tint_symbol(uint idx) {
  sb.arr[idx].scalar_f32 = 0.0f;
  sb.arr[idx].scalar_i32 = 0;
  sb.arr[idx].scalar_u32 = 0u;
  sb.arr[idx].scalar_f16 = 0.0hf;
  sb.arr[idx].vec2_f32 = vec2(0.0f);
  sb.arr[idx].vec2_i32 = ivec2(0);
  sb.arr[idx].vec2_u32 = uvec2(0u);
  sb.arr[idx].vec2_f16 = f16vec2(0.0hf);
  sb.arr[idx].vec3_f32 = vec3(0.0f);
  sb.arr[idx].vec3_i32 = ivec3(0);
  sb.arr[idx].vec3_u32 = uvec3(0u);
  sb.arr[idx].vec3_f16 = f16vec3(0.0hf);
  sb.arr[idx].vec4_f32 = vec4(0.0f);
  sb.arr[idx].vec4_i32 = ivec4(0);
  sb.arr[idx].vec4_u32 = uvec4(0u);
  sb.arr[idx].vec4_f16 = f16vec4(0.0hf);
  sb.arr[idx].mat2x2_f32 = mat2(vec2(0.0f), vec2(0.0f));
  sb.arr[idx].mat2x3_f32 = mat2x3(vec3(0.0f), vec3(0.0f));
  sb.arr[idx].mat2x4_f32 = mat2x4(vec4(0.0f), vec4(0.0f));
  sb.arr[idx].mat3x2_f32 = mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f));
  sb.arr[idx].mat3x3_f32 = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  sb.arr[idx].mat3x4_f32 = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  sb.arr[idx].mat4x2_f32 = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
  sb.arr[idx].mat4x3_f32 = mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
  sb.arr[idx].mat4x4_f32 = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  sb.arr[idx].mat2x2_f16 = f16mat2(f16vec2(0.0hf), f16vec2(0.0hf));
  sb.arr[idx].mat2x3_f16 = f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf));
  sb.arr[idx].mat2x4_f16 = f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf));
  sb.arr[idx].mat3x2_f16 = f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf));
  sb.arr[idx].mat3x3_f16 = f16mat3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf));
  sb.arr[idx].mat3x4_f16 = f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf));
  sb.arr[idx].mat4x2_f16 = f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf));
  sb.arr[idx].mat4x3_f16 = f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf));
  sb.arr[idx].mat4x4_f16 = f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf));
  uint tint_symbol_1[1] = uint[1](idx);
  vec3 tint_symbol_2[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  assign_and_preserve_padding_sb_arr_X_arr2_vec3_f32(tint_symbol_1, tint_symbol_2);
  f16mat4x2 tint_symbol_3[2] = f16mat4x2[2](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  sb.arr[idx].arr2_mat4x2_f16 = tint_symbol_3;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
