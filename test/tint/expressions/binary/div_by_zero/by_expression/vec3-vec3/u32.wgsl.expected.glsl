#version 310 es

uvec3 tint_div(uvec3 lhs, uvec3 rhs) {
  return (lhs / mix(rhs, uvec3(1u), equal(rhs, uvec3(0u))));
}

void f() {
  uvec3 a = uvec3(1u, 2u, 3u);
  uvec3 b = uvec3(0u, 5u, 0u);
  uvec3 r = tint_div(a, (b + b));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
