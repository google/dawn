#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int f() {
  int i = 0;
  while (true) {
    i = (i + 1);
    if ((i > 4)) {
      return i;
    }
  }
}

