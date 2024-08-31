#version 310 es

struct _A {
  int _B;
};

int s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  _A c = _A(0);
  int d = c._B;
  s = (c._B + d);
}
