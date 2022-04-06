#version 310 es
precision mediump float;

struct S {
  mat3x2 matrix;
  vec3 vector;
};

layout(binding = 0) uniform S_1 {
  mat3x2 matrix;
  vec3 vector;
} data;

void tint_symbol() {
  vec2 x = (data.matrix * data.vector);
}

void main() {
  tint_symbol();
  return;
}
