#version 310 es

struct S {
  mat4 matrix_view;
  mat3 matrix_normal;
};

layout(binding = 0, std140) uniform tint_symbol_block_ubo {
  S inner;
} tint_symbol;

vec4 tint_symbol_1() {
  float x = tint_symbol.inner.matrix_view[0].z;
  return vec4(x, 0.0f, 0.0f, 1.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = tint_symbol_1();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
