#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
bool t = false;
bvec2 m() {
  t = true;
  return bvec2(t);
}

void f() {
  bvec2 tint_symbol = m();
  uvec2 v = uvec2(tint_symbol);
}

