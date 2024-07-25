#version 310 es

layout(binding = 0, std140) uniform S_block_ubo {
  int inner;
} S;

int func_S_inner() {
  return S.inner;
}

void tint_symbol() {
  int r = func_S_inner();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
