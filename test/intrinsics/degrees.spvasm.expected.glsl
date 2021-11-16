#version 310 es
precision mediump float;

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  a = 42.0f;
  b = (a * 57.295780182f);
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


