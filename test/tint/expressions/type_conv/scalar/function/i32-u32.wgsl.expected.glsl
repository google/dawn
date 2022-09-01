#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int t = 0;
int m() {
  t = 1;
  return int(t);
}

void f() {
  int tint_symbol = m();
  uint v = uint(tint_symbol);
}

