#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

int A = 1;
int _A = 2;
void f() {
  int B = A;
  int _B = _A;
  s.inner = (B + _B);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
