#version 310 es

struct Inner {
  float scalar_f32;
  uint pad;
  uint pad_1;
  uint pad_2;
  vec3 vec3_f32;
  uint pad_3;
  mat2x4 mat2x4_f32;
};

struct S {
  Inner inner;
};

layout(binding = 0, std140) uniform u_block_ubo {
  S inner;
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  S inner;
} s;

void assign_and_preserve_padding_1_s_inner_inner(Inner value) {
  s.inner.inner.scalar_f32 = value.scalar_f32;
  s.inner.inner.vec3_f32 = value.vec3_f32;
  s.inner.inner.mat2x4_f32 = value.mat2x4_f32;
}

void assign_and_preserve_padding_s_inner(S value) {
  assign_and_preserve_padding_1_s_inner_inner(value.inner);
}

void tint_symbol() {
  S x = u.inner;
  assign_and_preserve_padding_s_inner(x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
