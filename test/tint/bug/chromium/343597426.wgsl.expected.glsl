#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void foo(bool a, bool b, bool c, bool d, bool e) {
  if (a) {
    if (b) {
      return;
    }
    if (c) {
      if (d) {
        return;
      }
      if (e) {
      }
    }
  }
}

