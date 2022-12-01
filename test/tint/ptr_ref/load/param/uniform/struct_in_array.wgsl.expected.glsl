#version 310 es

struct str {
  ivec4 i;
};

layout(binding = 0, std140) uniform S_block_ubo {
  str inner[4];
} S;

str func_S_X(uint pointer[1]) {
  return S.inner[pointer[0]];
}

void tint_symbol() {
  uint tint_symbol_1[1] = uint[1](2u);
  str r = func_S_X(tint_symbol_1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
