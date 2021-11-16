#version 310 es
precision mediump float;


layout (binding = 0) buffer SSBO_1 {
  mat2 m;
} ssbo;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f() {
  mat2 v = ssbo.m;
  ssbo.m = v;
  return;
}
void main() {
  f();
}


