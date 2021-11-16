#version 310 es
precision mediump float;

struct S {
  float a;
};

const bool bool_let = false;
const int i32_let = 0;
const uint u32_let = 0u;
const float f32_let = 0.0f;
const ivec2 v2i32_let = ivec2(0, 0);
const uvec3 v3u32_let = uvec3(0u, 0u, 0u);
const vec4 v4f32_let = vec4(0.0f, 0.0f, 0.0f, 0.0f);
const mat3x4 m3x4_let = mat3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
const float arr_let[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
const S struct_let = S(0.0f);

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  return;
}
void main() {
  tint_symbol();
}


