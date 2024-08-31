#version 310 es

int s;
void f(int _a) {
  int b = _a;
  s = b;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(1);
}
