#version 310 es
precision mediump float;

layout(binding = 0, std140) uniform S_ubo {
  mat3 matrix;
  vec3 vector;
  uint pad;
} data;

void tint_symbol() {
  vec3 x = (data.vector * data.matrix);
}

void main() {
  tint_symbol();
  return;
}
