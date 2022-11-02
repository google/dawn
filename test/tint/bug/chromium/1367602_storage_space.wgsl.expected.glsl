#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
layout(binding = 0, std430) buffer v_block_ssbo {
  int inner[1000000];
} v;

struct A {
  float a[1000000];
};

layout(binding = 1, std430) buffer b_block_ssbo {
  A inner;
} b;

