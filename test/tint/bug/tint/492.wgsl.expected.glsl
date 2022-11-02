#version 310 es

struct S {
  int a;
};

layout(binding = 0, std430) buffer buf_block_ssbo {
  S inner;
} buf;

void tint_symbol() {
  buf.inner.a = 12;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
