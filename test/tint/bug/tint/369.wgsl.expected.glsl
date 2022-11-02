#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  mat2 m;
};

struct S_std140 {
  vec2 m_0;
  vec2 m_1;
};

layout(binding = 0, std430) buffer SSBO_block_ssbo {
  S inner;
} SSBO;

layout(binding = 0, std140) uniform SSBO_block_std140_ubo {
  S_std140 inner;
} UBO;

