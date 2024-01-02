#version 310 es

layout(binding = 0, std140) uniform a_block_ubo {
  mat4x3 inner[4];
} a;

layout(binding = 1, std430) buffer s_block_ssbo {
  float inner;
} s;

int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}

void f() {
  int tint_symbol = i();
  int p_a_i_save = tint_symbol;
  int tint_symbol_1 = i();
  int p_a_i_i_save = tint_symbol_1;
  mat4x3 l_a[4] = a.inner;
  mat4x3 l_a_i = a.inner[p_a_i_save];
  vec3 l_a_i_i = a.inner[p_a_i_save][p_a_i_i_save];
  s.inner = (((a.inner[p_a_i_save][p_a_i_i_save].x + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
