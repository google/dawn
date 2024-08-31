#version 310 es

uint a[];
void tint_symbol() {
  a[1] = (a[1] + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
