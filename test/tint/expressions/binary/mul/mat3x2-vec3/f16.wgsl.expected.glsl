#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;

struct S {
  f16mat3x2 matrix;
  uint pad;
  f16vec3 vector;
  uint pad_1;
  uint pad_2;
};

struct S_std140 {
  f16vec2 matrix_0;
  f16vec2 matrix_1;
  f16vec2 matrix_2;
  uint pad;
  f16vec3 vector;
  uint pad_1;
  uint pad_2;
};

layout(binding = 0, std140) uniform data_block_std140_ubo {
  S_std140 inner;
} data;

f16mat3x2 load_data_inner_matrix() {
  return f16mat3x2(data.inner.matrix_0, data.inner.matrix_1, data.inner.matrix_2);
}

void tint_symbol() {
  f16vec2 x = (load_data_inner_matrix() * data.inner.vector);
}

void main() {
  tint_symbol();
  return;
}
