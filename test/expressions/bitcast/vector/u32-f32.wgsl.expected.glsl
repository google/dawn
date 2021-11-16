#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f() {
  uvec3 a = uvec3(1u, 2u, 3u);
  vec3 b = uintBitsToFloat(a);
  return;
}
void main() {
  f();
}


