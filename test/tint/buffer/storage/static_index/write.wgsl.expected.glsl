#version 310 es

struct Inner {
  int scalar_i32;
  float scalar_f32;
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
  uint pad_8;
  uint pad_9;
};

layout(binding = 0, std430) buffer sb_block_ssbo {
  S inner;
} sb;

void assign_and_preserve_padding_sb_mat2x3_f32(mat2x3 value) {
  sb.inner.mat2x3_f32[0] = value[0u];
  sb.inner.mat2x3_f32[1] = value[1u];
}

void assign_and_preserve_padding_1_sb_mat3x3_f32(mat3 value) {
  sb.inner.mat3x3_f32[0] = value[0u];
  sb.inner.mat3x3_f32[1] = value[1u];
  sb.inner.mat3x3_f32[2] = value[2u];
}

void assign_and_preserve_padding_2_sb_mat4x3_f32(mat4x3 value) {
  sb.inner.mat4x3_f32[0] = value[0u];
  sb.inner.mat4x3_f32[1] = value[1u];
  sb.inner.mat4x3_f32[2] = value[2u];
  sb.inner.mat4x3_f32[3] = value[3u];
}

void assign_and_preserve_padding_3_sb_arr2_vec3_f32(vec3 value[2]) {
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      sb.inner.arr2_vec3_f32[i] = value[i];
    }
  }
}

void tint_symbol() {
  sb.inner.scalar_f32 = 0.0f;
  sb.inner.scalar_i32 = 0;
  sb.inner.scalar_u32 = 0u;
  sb.inner.vec2_f32 = vec2(0.0f);
  sb.inner.vec2_i32 = ivec2(0);
  sb.inner.vec2_u32 = uvec2(0u);
  sb.inner.vec3_f32 = vec3(0.0f);
  sb.inner.vec3_i32 = ivec3(0);
  sb.inner.vec3_u32 = uvec3(0u);
  sb.inner.vec4_f32 = vec4(0.0f);
  sb.inner.vec4_i32 = ivec4(0);
  sb.inner.vec4_u32 = uvec4(0u);
  sb.inner.mat2x2_f32 = mat2(vec2(0.0f), vec2(0.0f));
  assign_and_preserve_padding_sb_mat2x3_f32(mat2x3(vec3(0.0f), vec3(0.0f)));
  sb.inner.mat2x4_f32 = mat2x4(vec4(0.0f), vec4(0.0f));
  sb.inner.mat3x2_f32 = mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f));
  assign_and_preserve_padding_1_sb_mat3x3_f32(mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  sb.inner.mat3x4_f32 = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  sb.inner.mat4x2_f32 = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
  assign_and_preserve_padding_2_sb_mat4x3_f32(mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  sb.inner.mat4x4_f32 = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec3 tint_symbol_1[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  assign_and_preserve_padding_3_sb_arr2_vec3_f32(tint_symbol_1);
  Inner tint_symbol_2 = Inner(0, 0.0f);
  sb.inner.struct_inner = tint_symbol_2;
  Inner tint_symbol_3[4] = Inner[4](Inner(0, 0.0f), Inner(0, 0.0f), Inner(0, 0.0f), Inner(0, 0.0f));
  sb.inner.array_struct_inner = tint_symbol_3;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
