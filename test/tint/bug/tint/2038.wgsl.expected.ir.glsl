#version 310 es

uint tint_symbol[2];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  if (false) {
    tint_symbol[0] = 1u;
  }
  if (false) {
    tint_symbol[1] = 1u;
  }
}
