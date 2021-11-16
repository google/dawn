#version 310 es
precision mediump float;

void func(int value, inout int pointer) {
  pointer = value;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  int i = 123;
  func(123, i);
  return;
}
void main() {
  tint_symbol();
}


