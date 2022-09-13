#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  int i = 0;
  while (true) {
    if (true) {
      break;
    }
    {
    }
    {
      int tint_symbol[1] = int[1](1);
      i = (i + tint_symbol[0]);
    }
  }
}

