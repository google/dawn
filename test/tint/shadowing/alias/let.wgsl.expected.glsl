#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  {
    int a_1 = 0;
    int b = a_1;
  }
  int a_2 = 0;
  int b = a_2;
}

