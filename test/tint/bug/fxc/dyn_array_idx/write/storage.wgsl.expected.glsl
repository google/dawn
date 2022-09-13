#version 310 es

layout(binding = 0, std140) uniform UBO_ubo {
  int dynamic_idx;
  uint pad;
  uint pad_1;
  uint pad_2;
} ubo;

layout(binding = 2, std430) buffer Result_ssbo {
  int tint_symbol;
} result;

layout(binding = 1, std430) buffer SSBO_ssbo {
  int data[4];
} ssbo;

void f() {
  ssbo.data[ubo.dynamic_idx] = 1;
  result.tint_symbol = ssbo.data[3];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
