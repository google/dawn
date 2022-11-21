#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  ivec4 a;
};

layout(binding = 0, std430) buffer v_block_ssbo {
  S inner;
} v;

ivec4 tint_mod(ivec4 lhs, int rhs) {
  ivec4 r = ivec4(rhs);
  return (lhs % mix(r, ivec4(1), bvec4(uvec4(equal(r, ivec4(0))) | uvec4(bvec4(uvec4(equal(lhs, ivec4(-2147483648))) & uvec4(equal(r, ivec4(-1))))))));
}

void foo() {
  ivec4 tint_symbol = tint_mod(v.inner.a, 2);
  v.inner.a = tint_symbol;
}

