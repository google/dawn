#version 310 es

layout(binding = 0, std140) uniform S_ubo {
  int i;
  uint pad;
  uint pad_1;
  uint pad_2;
} u;

void tint_symbol() {
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
