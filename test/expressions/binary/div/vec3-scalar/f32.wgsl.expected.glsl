#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f() {
  vec3 a = vec3(1.0f, 2.0f, 3.0f);
  vec3 r = (a / 4.0f);
  return;
}
void main() {
  f();
}


