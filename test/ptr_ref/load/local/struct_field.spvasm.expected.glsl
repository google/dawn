#version 310 es
precision mediump float;

struct S {
  int i;
};

void main_1() {
  int i = 0;
  S V = S(0);
  int x_14 = V.i;
  i = x_14;
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


