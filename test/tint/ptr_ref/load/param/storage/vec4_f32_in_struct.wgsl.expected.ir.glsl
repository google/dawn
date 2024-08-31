#version 310 es

struct str {
  vec4 i;
};

str S;
vec4 func() {
  return S.i;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 r = func();
}
