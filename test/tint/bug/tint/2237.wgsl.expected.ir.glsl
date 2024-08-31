#version 310 es

uint tint_symbol;
uint foo() {
  return uint[4](0u, 1u, 2u, 4u)[tint_symbol];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v = uint[4](0u, 1u, 2u, 4u)[tint_symbol];
  tint_symbol = (v + foo());
}
