#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

struct A {
  int B;
};

struct _A {
  int _B;
};

void f() {
  _A c = _A(0);
  int d = c._B;
  s.inner = (c._B + d);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
