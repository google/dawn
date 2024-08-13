#version 310 es

struct Uniforms {
  uint i;
};

struct OuterS {
  mat2x4 m1;
};

layout(binding = 4, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

void tint_symbol() {
  OuterS s1 = OuterS(mat2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  s1.m1[uniforms.inner.i] = vec4(1.0f);
  s1.m1[uniforms.inner.i][uniforms.inner.i] = 1.0f;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
