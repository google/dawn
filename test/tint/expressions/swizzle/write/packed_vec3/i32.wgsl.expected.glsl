#version 310 es


struct S {
  ivec3 v;
  uint tint_pad_0;
};

layout(binding = 0, std430)
buffer U_block_1_ssbo {
  S inner;
} v_1;
void f() {
  v_1.inner.v = ivec3(1, 2, 3);
  v_1.inner.v[0u] = 1;
  v_1.inner.v[1u] = 2;
  v_1.inner.v[2u] = 3;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
