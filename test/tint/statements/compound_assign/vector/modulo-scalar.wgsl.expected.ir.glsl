#version 310 es


struct S {
  ivec4 a;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  S tint_symbol;
} v_1;
ivec4 tint_mod_v4i32(ivec4 lhs, ivec4 rhs) {
  bvec4 v_2 = equal(rhs, ivec4(0));
  bvec4 v_3 = equal(lhs, ivec4((-2147483647 - 1)));
  bvec4 v_4 = equal(rhs, ivec4(-1));
  uvec4 v_5 = uvec4(v_3);
  bvec4 v_6 = bvec4((v_5 & uvec4(v_4)));
  uvec4 v_7 = uvec4(v_2);
  ivec4 v_8 = mix(rhs, ivec4(1), bvec4((v_7 | uvec4(v_6))));
  return (lhs - ((lhs / v_8) * v_8));
}
void foo() {
  ivec4 v_9 = v_1.tint_symbol.a;
  v_1.tint_symbol.a = tint_mod_v4i32(v_9, ivec4(2));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
