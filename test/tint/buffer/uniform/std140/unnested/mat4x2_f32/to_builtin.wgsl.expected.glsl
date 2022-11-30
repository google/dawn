#version 310 es

layout(binding = 0, std140) uniform u_block_std140_ubo {
  vec2 inner_0;
  vec2 inner_1;
  vec2 inner_2;
  vec2 inner_3;
} u;

mat4x2 load_u_inner() {
  return mat4x2(u.inner_0, u.inner_1, u.inner_2, u.inner_3);
}

void f() {
  mat2x4 t = transpose(load_u_inner());
  float l = length(u.inner_1);
  float a = abs(u.inner_0.yx[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
