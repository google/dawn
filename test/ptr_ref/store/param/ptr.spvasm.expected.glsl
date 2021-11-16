#version 310 es
precision mediump float;

void func(int value, inout int pointer) {
  pointer = value;
  return;
}

void main_1() {
  int i = 0;
  i = 123;
  func(123, i);
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


