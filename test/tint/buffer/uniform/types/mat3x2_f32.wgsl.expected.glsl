#version 310 es

layout(binding = 0, std140) uniform u_block_std140_ubo {
  vec2 inner_0;
  vec2 inner_1;
  vec2 inner_2;
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  mat3x2 inner;
} s;

mat3x2 load_u_inner() {
  return mat3x2(u.inner_0, u.inner_1, u.inner_2);
}

void tint_symbol() {
  mat3x2 x = load_u_inner();
  s.inner = x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
