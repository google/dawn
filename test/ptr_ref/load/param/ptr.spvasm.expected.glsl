#version 310 es
precision mediump float;

int func(int value, inout int pointer) {
  int x_9 = pointer;
  return (value + x_9);
}

void main_1() {
  int i = 0;
  i = 123;
  int x_18 = func(i, i);
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


