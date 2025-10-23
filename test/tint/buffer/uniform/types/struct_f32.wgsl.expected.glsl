#version 310 es


struct Inner {
  float scalar_f32;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec3 vec3_f32;
  uint tint_pad_3;
  mat2x4 mat2x4_f32;
};

struct S {
  Inner inner;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  S inner;
} v_1;
void tint_store_and_preserve_padding_1(Inner value_param) {
  v_1.inner.inner.scalar_f32 = value_param.scalar_f32;
  v_1.inner.inner.vec3_f32 = value_param.vec3_f32;
  v_1.inner.inner.mat2x4_f32 = value_param.mat2x4_f32;
}
void tint_store_and_preserve_padding(S value_param) {
  tint_store_and_preserve_padding_1(value_param.inner);
}
mat2x4 v_2(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
Inner v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  vec3 v_5 = uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz);
  return Inner(uintBitsToFloat(v_4[((start_byte_offset & 15u) >> 2u)]), 0u, 0u, 0u, v_5, 0u, v_2((32u + start_byte_offset)));
}
S v_6(uint start_byte_offset) {
  return S(v_3(start_byte_offset));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S x = v_6(0u);
  tint_store_and_preserve_padding(x);
}
