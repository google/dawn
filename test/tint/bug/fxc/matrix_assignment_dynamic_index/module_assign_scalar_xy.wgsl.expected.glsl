#version 310 es

struct Uniforms {
  uint i;
  uint j;
  uint pad;
  uint pad_1;
};

layout(binding = 4, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

mat2x4 m1 = mat2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
void tint_symbol() {
  m1[uniforms.inner.i][uniforms.inner.j] = 1.0f;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
