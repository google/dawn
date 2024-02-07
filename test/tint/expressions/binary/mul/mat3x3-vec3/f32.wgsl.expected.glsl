#version 310 es
precision highp float;
precision highp int;

struct S {
  mat3 matrix;
  vec3 vector;
  uint pad;
};

layout(binding = 0, std140) uniform data_block_ubo {
  S inner;
} data;

void tint_symbol() {
  vec3 x = (data.inner.matrix * data.inner.vector);
}

void main() {
  tint_symbol();
  return;
}
