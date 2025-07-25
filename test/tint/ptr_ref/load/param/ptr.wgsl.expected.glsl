#version 310 es

int func(int value, inout int pointer) {
  int v = pointer;
  uint v_1 = uint(value);
  return int((v_1 + uint(v)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = 123;
  int r = func(i, i);
}
