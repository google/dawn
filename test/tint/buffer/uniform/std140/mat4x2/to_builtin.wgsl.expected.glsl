#version 310 es

struct S {
  int before;
  mat4x2 m;
  int after;
};

struct S_std140 {
  int before;
  uint pad;
  vec2 m_0;
  vec2 m_1;
  vec2 m_2;
  vec2 m_3;
  int after;
  uint pad_1;
};

layout(binding = 0, std140) uniform u_block_ubo {
  S_std140 inner[4];
} u;

mat4x2 load_u_2_m() {
  return mat4x2(u.inner[2u].m_0, u.inner[2u].m_1, u.inner[2u].m_2, u.inner[2u].m_3);
}

void f() {
  mat2x4 t = transpose(load_u_2_m());
  float l = length(u.inner[0u].m_1.yx);
  float a = abs(u.inner[0u].m_1.yx[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
