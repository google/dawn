#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  int i = 123;
  int use = (i + 1);
  return;
}
void main() {
  tint_symbol();
}


