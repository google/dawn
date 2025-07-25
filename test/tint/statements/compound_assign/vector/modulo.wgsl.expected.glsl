#version 310 es


struct S {
  ivec4 a;
};

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  S inner;
} v_1;
ivec4 tint_mod_v4i32(ivec4 lhs, ivec4 rhs) {
  uvec4 v_2 = uvec4(equal(lhs, ivec4((-2147483647 - 1))));
  bvec4 v_3 = bvec4((v_2 & uvec4(equal(rhs, ivec4(-1)))));
  uvec4 v_4 = uvec4(equal(rhs, ivec4(0)));
  ivec4 v_5 = mix(rhs, ivec4(1), bvec4((v_4 | uvec4(v_3))));
  uvec4 v_6 = uvec4((lhs / v_5));
  ivec4 v_7 = ivec4((v_6 * uvec4(v_5)));
  uvec4 v_8 = uvec4(lhs);
  return ivec4((v_8 - uvec4(v_7)));
}
void foo() {
  v_1.inner.a = tint_mod_v4i32(v_1.inner.a, ivec4(2));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
