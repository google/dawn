#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int f() {
  int i = 0;
  while((i < 4)) {
    i = (i + 1);
    continue;
  }
  return i;
}

