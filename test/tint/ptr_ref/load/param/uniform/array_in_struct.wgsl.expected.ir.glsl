#version 310 es

struct str {
  ivec4 arr[4];
};

uniform str S;
ivec4[4] func() {
  return S.arr;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec4 r[4] = func();
}
