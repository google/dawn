#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

struct a {
  int b;
};

struct _a {
  int _b;
};

void f() {
  _a c = _a(0);
  int d = c._b;
  s.inner = (c._b + d);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
