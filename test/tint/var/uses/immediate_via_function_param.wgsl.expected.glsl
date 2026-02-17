#version 310 es

layout(location = 0) uniform uint tint_immediates[1];
layout(binding = 0, std430)
buffer output_block_1_ssbo {
  uint inner;
} v;
void foo() {
  v.inner = tint_immediates[0u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo();
}
