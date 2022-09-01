#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uint t = 0u;
uvec2 m() {
  t = 1u;
  return uvec2(t);
}

void f() {
  uvec2 tint_symbol = m();
  bvec2 v = bvec2(tint_symbol);
}

