#version 310 es

void func(inout int pointer) {
  pointer = 42;
}

void tint_symbol() {
  int F = 0;
  func(F);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
