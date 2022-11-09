#version 310 es

uvec3 tint_div(uvec3 lhs, uint rhs) {
  uvec3 r = uvec3(rhs);
  return (lhs / mix(r, uvec3(1u), equal(r, uvec3(0u))));
}

void f() {
  uvec3 a = uvec3(1u, 2u, 3u);
  uint b = 4u;
  uvec3 r = tint_div(a, b);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
