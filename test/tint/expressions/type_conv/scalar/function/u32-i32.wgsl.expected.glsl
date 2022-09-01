#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uint t = 0u;
uint m() {
  t = 1u;
  return uint(t);
}

void f() {
  uint tint_symbol = m();
  int v = int(tint_symbol);
}

