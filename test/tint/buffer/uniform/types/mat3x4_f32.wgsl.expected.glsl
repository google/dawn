#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat3x4 inner;
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  mat3x4 inner;
} s;

void tint_symbol() {
  mat3x4 x = u.inner;
  s.inner = x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
