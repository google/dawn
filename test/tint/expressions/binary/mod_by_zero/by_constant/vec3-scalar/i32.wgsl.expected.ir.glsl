#version 310 es

ivec3 tint_mod_v3i32(ivec3 lhs, ivec3 rhs) {
  uvec3 v = uvec3((lhs == ivec3((-2147483647 - 1))));
  bvec3 v_1 = bvec3((v & uvec3((rhs == ivec3(-1)))));
  uvec3 v_2 = uvec3((rhs == ivec3(0)));
  bvec3 v_3 = bvec3((v_2 | uvec3(v_1)));
  int v_4 = ((v_3.x) ? (ivec3(1).x) : (rhs.x));
  int v_5 = ((v_3.y) ? (ivec3(1).y) : (rhs.y));
  ivec3 v_6 = ivec3(v_4, v_5, ((v_3.z) ? (ivec3(1).z) : (rhs.z)));
  return (lhs - ((lhs / v_6) * v_6));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec3 a = ivec3(1, 2, 3);
  int b = 0;
  ivec3 r = tint_mod_v3i32(a, ivec3(b));
}
