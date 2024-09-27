#version 310 es

uint tint_mod_u32(uint lhs, uint rhs) {
  uint v_1 = mix(rhs, 1u, (rhs == 0u));
  return (lhs - ((lhs / v_1) * v_1));
}
void f_inner(uvec3 v) {
  uint l = (v[0u] << (tint_mod_u32(v[1u], 1u) & 31u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_GlobalInvocationID);
}
