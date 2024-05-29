#version 310 es

layout(binding = 0, std430) buffer S_block_ssbo {
  mat2x4 inner;
} S;

void func_S_inner_X(uint pointer[1]) {
  S.inner[pointer[0]] = vec4(0.0f);
}

void tint_symbol() {
  uint tint_symbol_1[1] = uint[1](1u);
  func_S_inner_X(tint_symbol_1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
