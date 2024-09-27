#version 310 es

uvec3 tint_mod(uint lhs, uvec3 rhs) {
  uvec3 l = uvec3(lhs);
  return (l % mix(rhs, uvec3(1u), equal(rhs, uvec3(0u))));
}

void f() {
  uint a = 4u;
  uvec3 b = uvec3(1u, 2u, 3u);
  uvec3 r = tint_mod(a, b);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
