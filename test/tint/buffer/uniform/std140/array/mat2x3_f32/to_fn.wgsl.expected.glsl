#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat2x3 inner[4];
} u;

layout(binding = 1, std430) buffer s_block_ssbo {
  float inner;
} s;

float a(mat2x3 a_1[4]) {
  return a_1[0][0].x;
}

float b(mat2x3 m) {
  return m[0].x;
}

float c(vec3 v) {
  return v.x;
}

float d(float f_1) {
  return f_1;
}

void f() {
  float tint_symbol = a(u.inner);
  float tint_symbol_1 = b(u.inner[1]);
  float tint_symbol_2 = c(u.inner[1][0].zxy);
  float tint_symbol_3 = d(u.inner[1][0].zxy.x);
  s.inner = (((tint_symbol + tint_symbol_1) + tint_symbol_2) + tint_symbol_3);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
