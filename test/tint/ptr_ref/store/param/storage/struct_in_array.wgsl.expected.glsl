#version 310 es

struct str {
  int i;
};

layout(binding = 0, std430) buffer S_block_ssbo {
  str inner[4];
} S;

void func_S_inner_X(uint pointer[1]) {
  str tint_symbol_1 = str(0);
  S.inner[pointer[0]] = tint_symbol_1;
}

void tint_symbol() {
  uint tint_symbol_2[1] = uint[1](2u);
  func_S_inner_X(tint_symbol_2);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
