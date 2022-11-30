#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat4 inner[4];
} u;

void f() {
  mat4 t = transpose(u.inner[2]);
  float l = length(u.inner[0][1].ywxz);
  float a = abs(u.inner[0][1].ywxz.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
