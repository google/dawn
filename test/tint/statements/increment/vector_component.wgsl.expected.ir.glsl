#version 310 es

uvec4 a;
void tint_symbol() {
  a[1] = (a.y + 1u);
  a[2u] = (a.z + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
