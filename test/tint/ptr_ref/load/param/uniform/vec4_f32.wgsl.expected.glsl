#version 310 es

layout(binding = 0, std140) uniform S_block_ubo {
  vec4 inner;
} S;

vec4 func_S() {
  return S.inner;
}

void tint_symbol() {
  vec4 r = func_S();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
