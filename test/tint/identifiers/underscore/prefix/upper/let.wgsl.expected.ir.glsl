#version 310 es

int s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int A = 1;
  int _A = 2;
  int B = A;
  int _B = _A;
  s = (((A + _A) + B) + _B);
}
