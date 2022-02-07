#version 310 es

int func(int value, inout int pointer) {
  return (value + pointer);
}

void tint_symbol() {
  int i = 123;
  int r = func(i, i);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
