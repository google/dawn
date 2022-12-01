#version 310 es

struct S {
  vec4 a;
  int b;
  uint pad;
  uint pad_1;
  uint pad_2;
};

layout(binding = 0, std430) buffer sb_block_ssbo {
  S inner[];
} sb;

void tint_symbol() {
  S x = sb.inner[1];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
