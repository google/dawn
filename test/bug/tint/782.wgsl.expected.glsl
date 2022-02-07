#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void foo() {
  int explicit[2] = int[2](0, 0);
  int implict[2] = int[2](0, 0);
  implict = explicit;
}

