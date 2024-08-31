#version 310 es

int s;
int A = 1;
int _A = 2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int B = A;
  int _B = _A;
  s = (B + _B);
}
