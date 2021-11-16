#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f() {
  float b = 1.0f;
  return;
}
void main() {
  f();
}


