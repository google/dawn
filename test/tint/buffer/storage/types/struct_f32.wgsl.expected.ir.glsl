#version 310 es


struct Inner {
  float scalar_f32;
  vec3 vec3_f32;
  mat2x4 mat2x4_f32;
};

struct S {
  Inner inner;
};

layout(binding = 0, std430)
buffer tint_symbol_4_1_ssbo {
  S tint_symbol_3;
} v;
layout(binding = 1, std430)
buffer tint_symbol_6_1_ssbo {
  S tint_symbol_5;
} v_1;
void tint_store_and_preserve_padding_1(inout Inner target, Inner value_param) {
  target.scalar_f32 = value_param.scalar_f32;
  target.vec3_f32 = value_param.vec3_f32;
  target.mat2x4_f32 = value_param.mat2x4_f32;
}
void tint_store_and_preserve_padding(inout S target, S value_param) {
  tint_store_and_preserve_padding_1(target.inner, value_param.inner);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S t = v.tint_symbol_3;
  tint_store_and_preserve_padding(v_1.tint_symbol_5, t);
}
