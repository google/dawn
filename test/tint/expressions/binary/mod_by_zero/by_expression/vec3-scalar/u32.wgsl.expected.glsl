#version 310 es

uvec3 tint_mod(uvec3 lhs, uint rhs) {
  uvec3 r = uvec3(rhs);
  return (lhs % mix(r, uvec3(1u), equal(r, uvec3(0u))));
}

void f() {
  uvec3 a = uvec3(1u, 2u, 3u);
  uint b = 0u;
  uvec3 r = tint_mod(a, (b + b));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
