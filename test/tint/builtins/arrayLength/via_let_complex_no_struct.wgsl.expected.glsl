#version 310 es

layout(binding = 0, std430) buffer G_block_ssbo {
  int inner[];
} G;

void tint_symbol() {
  uint l1 = uint(G.inner.length());
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
