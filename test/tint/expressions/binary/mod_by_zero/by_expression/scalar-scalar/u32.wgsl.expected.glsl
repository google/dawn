#version 310 es

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % ((rhs == 0u) ? 1u : rhs));
}

void f() {
  uint a = 1u;
  uint b = 0u;
  uint r = tint_mod(a, (b + b));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
