#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int f() {
  int i = 0;
  int j = 0;
  while (true) {
    if ((i > 4)) {
      return 1;
    }
    while (true) {
      if ((j > 4)) {
        return 2;
      }
      {
        j = (j + 1);
      }
    }
    {
      i = (i + 1);
    }
  }
}

