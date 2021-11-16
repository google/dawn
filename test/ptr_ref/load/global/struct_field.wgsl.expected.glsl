#version 310 es
precision mediump float;

struct S {
  int i;
};

S V = S(0);

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  int i = V.i;
  return;
}
void main() {
  tint_symbol();
}


