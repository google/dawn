#version 310 es


struct Inner {
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
};

layout(binding = 0, std430)
buffer S_1_ssbo {
  Inner arr[];
} sb;
void tint_store_and_preserve_padding_3(uint target_indices[1], vec3 value_param[2]) {
  {
    uint v = 0u;
    v = 0u;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 2u)) {
        break;
      }
      sb.arr[target_indices[0u]].arr2_vec3_f32[v_1] = value_param[v_1];
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_2(uint target_indices[1], mat4x3 value_param) {
  sb.arr[target_indices[0u]].mat4x3_f32[0u] = value_param[0u];
  sb.arr[target_indices[0u]].mat4x3_f32[1u] = value_param[1u];
  sb.arr[target_indices[0u]].mat4x3_f32[2u] = value_param[2u];
  sb.arr[target_indices[0u]].mat4x3_f32[3u] = value_param[3u];
}
void tint_store_and_preserve_padding_1(uint target_indices[1], mat3 value_param) {
  sb.arr[target_indices[0u]].mat3x3_f32[0u] = value_param[0u];
  sb.arr[target_indices[0u]].mat3x3_f32[1u] = value_param[1u];
  sb.arr[target_indices[0u]].mat3x3_f32[2u] = value_param[2u];
}
void tint_store_and_preserve_padding(uint target_indices[1], mat2x3 value_param) {
  sb.arr[target_indices[0u]].mat2x3_f32[0u] = value_param[0u];
  sb.arr[target_indices[0u]].mat2x3_f32[1u] = value_param[1u];
}
void tint_symbol_inner(uint idx) {
  sb.arr[idx].scalar_f32 = 0.0f;
  sb.arr[idx].scalar_i32 = 0;
  sb.arr[idx].scalar_u32 = 0u;
  sb.arr[idx].vec2_f32 = vec2(0.0f);
  sb.arr[idx].vec2_i32 = ivec2(0);
  sb.arr[idx].vec2_u32 = uvec2(0u);
  sb.arr[idx].vec3_f32 = vec3(0.0f);
  sb.arr[idx].vec3_i32 = ivec3(0);
  sb.arr[idx].vec3_u32 = uvec3(0u);
  sb.arr[idx].vec4_f32 = vec4(0.0f);
  sb.arr[idx].vec4_i32 = ivec4(0);
  sb.arr[idx].vec4_u32 = uvec4(0u);
  sb.arr[idx].mat2x2_f32 = mat2(vec2(0.0f), vec2(0.0f));
  tint_store_and_preserve_padding(uint[1](idx), mat2x3(vec3(0.0f), vec3(0.0f)));
  sb.arr[idx].mat2x4_f32 = mat2x4(vec4(0.0f), vec4(0.0f));
  sb.arr[idx].mat3x2_f32 = mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f));
  tint_store_and_preserve_padding_1(uint[1](idx), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  sb.arr[idx].mat3x4_f32 = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  sb.arr[idx].mat4x2_f32 = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
  tint_store_and_preserve_padding_2(uint[1](idx), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  sb.arr[idx].mat4x4_f32 = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  tint_store_and_preserve_padding_3(uint[1](idx), vec3[2](vec3(0.0f), vec3(0.0f)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationIndex);
}
