#version 310 es

ivec3 tint_mod_v3i32(ivec3 lhs, ivec3 rhs) {
  bvec3 v = equal(rhs, ivec3(0));
  bvec3 v_1 = equal(lhs, ivec3((-2147483647 - 1)));
  bvec3 v_2 = equal(rhs, ivec3(-1));
  uvec3 v_3 = uvec3(v_1);
  bvec3 v_4 = bvec3((v_3 & uvec3(v_2)));
  uvec3 v_5 = uvec3(v);
  ivec3 v_6 = mix(rhs, ivec3(1), bvec3((v_5 | uvec3(v_4))));
  return (lhs - ((lhs / v_6) * v_6));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec3 a = ivec3(1, 2, 3);
  ivec3 b = ivec3(0, 5, 0);
  ivec3 r = tint_mod_v3i32(a, (b + b));
}
