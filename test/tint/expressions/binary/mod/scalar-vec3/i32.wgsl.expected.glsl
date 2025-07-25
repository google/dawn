#version 310 es

ivec3 tint_mod_v3i32(ivec3 lhs, ivec3 rhs) {
  uvec3 v = uvec3(equal(lhs, ivec3((-2147483647 - 1))));
  bvec3 v_1 = bvec3((v & uvec3(equal(rhs, ivec3(-1)))));
  uvec3 v_2 = uvec3(equal(rhs, ivec3(0)));
  ivec3 v_3 = mix(rhs, ivec3(1), bvec3((v_2 | uvec3(v_1))));
  uvec3 v_4 = uvec3((lhs / v_3));
  ivec3 v_5 = ivec3((v_4 * uvec3(v_3)));
  uvec3 v_6 = uvec3(lhs);
  return ivec3((v_6 - uvec3(v_5)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 4;
  ivec3 b = ivec3(1, 2, 3);
  ivec3 r = tint_mod_v3i32(ivec3(a), b);
}
