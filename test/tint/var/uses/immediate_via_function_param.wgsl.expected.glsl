#version 310 es


struct tint_immediate_struct {
  uint inner;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
layout(binding = 0, std430)
buffer output_block_1_ssbo {
  uint inner;
} v;
void foo() {
  v.inner = tint_immediates.inner;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo();
}
