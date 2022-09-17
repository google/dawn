#version 310 es

struct S {
  int before;
  mat4x2 m;
  int after;
};

layout(binding = 0, std140) uniform u_block_ubo {
  mat4x2 inner[4];
} u;

mat4x2 p[4] = mat4x2[4](mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
void f() {
  p = u.inner;
  p[1] = u.inner[2];
  p[1][0] = u.inner[0][1].yx;
  p[1][0].x = u.inner[0][1].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
