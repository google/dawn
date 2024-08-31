#version 310 es

struct str {
  int arr[4];
};

uniform str S;
int[4] func() {
  return S.arr;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int r[4] = func();
}
