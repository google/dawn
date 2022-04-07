#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void tint_symbol() {
  int i = 0;
  i = (i - 1);
}

