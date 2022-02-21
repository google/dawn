#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void some_loop_body() {
}

void f() {
  int j = 0;
  {
    int i = 0;
    while (true) {
      bool tint_tmp = (i < 5);
      if (tint_tmp) {
        tint_tmp = (j < 10);
      }
      if (!((tint_tmp))) { break; }
      some_loop_body();
      j = (i * 30);
      i = (i + 1);
    }
  }
}

