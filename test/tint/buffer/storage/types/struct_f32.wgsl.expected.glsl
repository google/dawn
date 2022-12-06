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

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  S inner;
} tint_symbol;

layout(binding = 1, std430) buffer tint_symbol_block_ssbo_1 {
  S inner;
} tint_symbol_1;

void assign_and_preserve_padding_1_tint_symbol_1_inner(Inner value) {
  tint_symbol_1.inner.inner.scalar_f32 = value.scalar_f32;
  tint_symbol_1.inner.inner.vec3_f32 = value.vec3_f32;
  tint_symbol_1.inner.inner.mat2x4_f32 = value.mat2x4_f32;
}

void assign_and_preserve_padding_tint_symbol_1(S value) {
  assign_and_preserve_padding_1_tint_symbol_1_inner(value.inner);
}

void tint_symbol_2() {
  S t = tint_symbol.inner;
  assign_and_preserve_padding_tint_symbol_1(t);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_2();
  return;
}
