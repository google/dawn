#version 310 es

struct Uniforms {
  uint i;
  uint pad;
  uint pad_1;
  uint pad_2;
};

struct InnerS {
  int v;
};

layout(binding = 4, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

layout(binding = 0, std430) buffer OuterS_ssbo {
  InnerS a1[];
} s1;

void tint_symbol() {
  InnerS v = InnerS(0);
  s1.a1[uniforms.inner.i] = v;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
