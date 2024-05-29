#version 310 es

struct str {
  int i;
};

layout(binding = 0, std430) buffer S_block_ssbo {
  str inner;
} S;

void func_S_inner_i() {
  S.inner.i = 42;
}

void tint_symbol() {
  func_S_inner_i();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
