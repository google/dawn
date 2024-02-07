#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

struct S {
  f16mat3 matrix;
  f16vec3 vector;
};

struct S_std140 {
  f16vec3 matrix_0;
  f16vec3 matrix_1;
  f16vec3 matrix_2;
  f16vec3 vector;
};

layout(binding = 0, std140) uniform data_block_std140_ubo {
  S_std140 inner;
} data;

f16mat3 load_data_inner_matrix() {
  return f16mat3(data.inner.matrix_0, data.inner.matrix_1, data.inner.matrix_2);
}

void tint_symbol() {
  f16vec3 x = (load_data_inner_matrix() * data.inner.vector);
}

void main() {
  tint_symbol();
  return;
}
