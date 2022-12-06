#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct Inner {
  float16_t scalar_f16;
  uint pad;
  f16vec3 vec3_f16;
  f16mat2x4 mat2x4_f16;
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
  tint_symbol_1.inner.inner.scalar_f16 = value.scalar_f16;
  tint_symbol_1.inner.inner.vec3_f16 = value.vec3_f16;
  tint_symbol_1.inner.inner.mat2x4_f16 = value.mat2x4_f16;
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
