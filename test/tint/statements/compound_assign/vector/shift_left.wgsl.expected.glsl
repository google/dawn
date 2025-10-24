#version 310 es


struct S {
  ivec4 a;
};

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  S inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner.a = ivec4((uvec4(v_1.inner.a) << (uvec4(2u) & uvec4(31u))));
}
