#version 310 es

vec4 S;
vec4 func() {
  return S;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 r = func();
}
