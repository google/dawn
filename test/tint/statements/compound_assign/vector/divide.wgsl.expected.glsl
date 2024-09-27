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

ivec4 tint_div(ivec4 lhs, ivec4 rhs) {
  return (lhs / mix(rhs, ivec4(1), bvec4(uvec4(equal(rhs, ivec4(0))) | uvec4(bvec4(uvec4(equal(lhs, ivec4((-2147483647 - 1)))) & uvec4(equal(rhs, ivec4(-1))))))));
}

void foo() {
  v.inner.a = tint_div(v.inner.a, ivec4(2));
}

