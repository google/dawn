#version 310 es

void func(int value, inout int pointer) {
  pointer = value;
}

void tint_symbol() {
  int i = 123;
  func(123, i);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
