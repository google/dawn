#version 310 es

int s;
void f(int _A) {
  int B = _A;
  s = B;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(1);
}
