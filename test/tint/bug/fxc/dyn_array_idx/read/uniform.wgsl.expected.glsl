#version 310 es

layout(binding = 0, std140) uniform UBO_ubo {
  ivec4 data[4];
  int dynamic_idx;
  uint pad;
  uint pad_1;
  uint pad_2;
} ubo;

layout(binding = 2, std430) buffer Result_ssbo {
  int tint_symbol;
} result;

void f() {
  result.tint_symbol = ubo.data[ubo.dynamic_idx].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
