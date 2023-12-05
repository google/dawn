#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

void f() {
  int c = 0;
  int d = 0;
  s.inner = (c + d);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
