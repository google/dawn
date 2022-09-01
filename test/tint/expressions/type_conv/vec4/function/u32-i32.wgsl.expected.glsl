#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uint t = 0u;
uvec4 m() {
  t = 1u;
  return uvec4(t);
}

void f() {
  uvec4 tint_symbol = m();
  ivec4 v = ivec4(tint_symbol);
}

