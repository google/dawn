#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat4 inner[4];
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  mat4 inner[4];
} s;

void f() {
  s.inner = u.inner;
  s.inner[1] = u.inner[2];
  s.inner[1][0] = u.inner[0][1].ywxz;
  s.inner[1][0].x = u.inner[0][1].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
