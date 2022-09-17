#version 310 es
precision mediump float;

layout(binding = 0, std140) uniform S_std140_ubo {
  vec2 matrix_0;
  vec2 matrix_1;
  vec2 matrix_2;
  uint pad;
  uint pad_1;
  vec3 vector;
  uint pad_2;
} data;

mat3x2 load_data_matrix() {
  return mat3x2(data.matrix_0, data.matrix_1, data.matrix_2);
}

void tint_symbol() {
  vec2 x = (load_data_matrix() * data.vector);
}

void main() {
  tint_symbol();
  return;
}
