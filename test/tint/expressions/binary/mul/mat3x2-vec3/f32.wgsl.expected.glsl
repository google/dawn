#version 310 es
precision highp float;

struct S {
  mat3x2 matrix;
  uint pad;
  uint pad_1;
  vec3 vector;
  uint pad_2;
};

struct S_std140 {
  vec2 matrix_0;
  vec2 matrix_1;
  vec2 matrix_2;
  uint pad;
  uint pad_1;
  vec3 vector;
  uint pad_2;
};

layout(binding = 0, std140) uniform data_block_std140_ubo {
  S_std140 inner;
} data;

mat3x2 load_data_inner_matrix() {
  return mat3x2(data.inner.matrix_0, data.inner.matrix_1, data.inner.matrix_2);
}

void tint_symbol() {
  vec2 x = (load_data_inner_matrix() * data.inner.vector);
}

void main() {
  tint_symbol();
  return;
}
