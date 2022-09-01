#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int t = 0;
ivec3 m() {
  t = 1;
  return ivec3(t);
}

void f() {
  ivec3 tint_symbol = m();
  uvec3 v = uvec3(tint_symbol);
}

