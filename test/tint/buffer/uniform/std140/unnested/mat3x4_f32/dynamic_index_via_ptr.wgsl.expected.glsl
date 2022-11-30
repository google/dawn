#version 310 es

layout(binding = 0, std140) uniform m_block_ubo {
  mat3x4 inner;
} m;

int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}

void f() {
  int tint_symbol = i();
  int p_m_i_save = tint_symbol;
  mat3x4 l_m = m.inner;
  vec4 l_m_i = m.inner[p_m_i_save];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
