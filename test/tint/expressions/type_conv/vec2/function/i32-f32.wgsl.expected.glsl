#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int t = 0;
ivec2 m() {
  t = 1;
  return ivec2(t);
}

void f() {
  ivec2 tint_symbol = m();
  vec2 v = vec2(tint_symbol);
}

