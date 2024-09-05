#version 310 es


struct Inner {
  int scalar_i32;
  float scalar_f32;
};

struct S {
  float scalar_f32;
  int scalar_i32;
  uint scalar_u32;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  vec3 vec3_f32;
  ivec3 vec3_i32;
  uvec3 vec3_u32;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  mat2 mat2x2_f32;
  mat2x3 mat2x3_f32;
  mat2x4 mat2x4_f32;
  mat3x2 mat3x2_f32;
  mat3 mat3x3_f32;
  mat3x4 mat3x4_f32;
  mat4x2 mat4x2_f32;
  mat4x3 mat4x3_f32;
  mat4 mat4x4_f32;
  vec3 arr2_vec3_f32[2];
  Inner struct_inner;
  Inner array_struct_inner[4];
};

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  S tint_symbol_1;
} v;
void tint_store_and_preserve_padding_3(inout vec3 target[2], vec3 value_param[2]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 2u)) {
        break;
      }
      target[v_2] = value_param[v_2];
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_2(inout mat4x3 target, mat4x3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
  target[2u] = value_param[2u];
  target[3u] = value_param[3u];
}
void tint_store_and_preserve_padding_1(inout mat3 target, mat3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
  target[2u] = value_param[2u];
}
void tint_store_and_preserve_padding(inout mat2x3 target, mat2x3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol_1.scalar_f32 = 0.0f;
  v.tint_symbol_1.scalar_i32 = 0;
  v.tint_symbol_1.scalar_u32 = 0u;
  v.tint_symbol_1.vec2_f32 = vec2(0.0f);
  v.tint_symbol_1.vec2_i32 = ivec2(0);
  v.tint_symbol_1.vec2_u32 = uvec2(0u);
  v.tint_symbol_1.vec3_f32 = vec3(0.0f);
  v.tint_symbol_1.vec3_i32 = ivec3(0);
  v.tint_symbol_1.vec3_u32 = uvec3(0u);
  v.tint_symbol_1.vec4_f32 = vec4(0.0f);
  v.tint_symbol_1.vec4_i32 = ivec4(0);
  v.tint_symbol_1.vec4_u32 = uvec4(0u);
  v.tint_symbol_1.mat2x2_f32 = mat2(vec2(0.0f), vec2(0.0f));
  tint_store_and_preserve_padding(v.tint_symbol_1.mat2x3_f32, mat2x3(vec3(0.0f), vec3(0.0f)));
  v.tint_symbol_1.mat2x4_f32 = mat2x4(vec4(0.0f), vec4(0.0f));
  v.tint_symbol_1.mat3x2_f32 = mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f));
  tint_store_and_preserve_padding_1(v.tint_symbol_1.mat3x3_f32, mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  v.tint_symbol_1.mat3x4_f32 = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  v.tint_symbol_1.mat4x2_f32 = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
  tint_store_and_preserve_padding_2(v.tint_symbol_1.mat4x3_f32, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  v.tint_symbol_1.mat4x4_f32 = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  tint_store_and_preserve_padding_3(v.tint_symbol_1.arr2_vec3_f32, vec3[2](vec3(0.0f), vec3(0.0f)));
  v.tint_symbol_1.struct_inner = Inner(0, 0.0f);
  v.tint_symbol_1.array_struct_inner = Inner[4](Inner(0, 0.0f), Inner(0, 0.0f), Inner(0, 0.0f), Inner(0, 0.0f));
}
