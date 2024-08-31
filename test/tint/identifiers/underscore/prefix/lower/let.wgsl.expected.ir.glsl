#version 310 es

int s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 1;
  int _a = a;
  int b = a;
  int _b = _a;
  s = (((a + _a) + b) + _b);
}
