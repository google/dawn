#version 310 es

layout(binding = 0, std140) uniform a_block_ubo {
  mat3 inner[4];
} a;

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
  mat3 l_a[4] = a.inner;
  mat3 l_a_i = a.inner[p_a_i_save];
  vec3 l_a_i_i = a.inner[p_a_i_save][p_a_i_i_save];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
