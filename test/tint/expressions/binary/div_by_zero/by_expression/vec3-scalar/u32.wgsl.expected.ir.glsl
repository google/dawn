#version 310 es

uvec3 tint_div_v3u32(uvec3 lhs, uvec3 rhs) {
  bvec3 v = equal(rhs, uvec3(0u));
  uint v_1 = ((v.x) ? (uvec3(1u).x) : (rhs.x));
  uint v_2 = ((v.y) ? (uvec3(1u).y) : (rhs.y));
  return (lhs / uvec3(v_1, v_2, ((v.z) ? (uvec3(1u).z) : (rhs.z))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec3 a = uvec3(1u, 2u, 3u);
  uint b = 0u;
  uvec3 v_3 = a;
  uvec3 r = tint_div_v3u32(v_3, uvec3((b + b)));
}
