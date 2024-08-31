#version 310 es

struct str {
  int arr[4];
};

str S;
void func() {
  S.arr = int[4](0, 0, 0, 0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func();
}
