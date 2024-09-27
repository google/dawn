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

ivec4 tint_mod(ivec4 lhs, ivec4 rhs) {
  ivec4 rhs_or_one = mix(rhs, ivec4(1), bvec4(uvec4(equal(rhs, ivec4(0))) | uvec4(bvec4(uvec4(equal(lhs, ivec4((-2147483647 - 1)))) & uvec4(equal(rhs, ivec4(-1)))))));
  if (any(notEqual((uvec4((lhs | rhs_or_one)) & uvec4(2147483648u)), uvec4(0u)))) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

void foo() {
  v.inner.a = tint_mod(v.inner.a, ivec4(2));
}

