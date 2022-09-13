#version 310 es

struct S {
  int before;
  mat3x2 m;
  int after;
};

struct S_std140 {
  int before;
  uint pad;
  vec2 m_0;
  vec2 m_1;
  vec2 m_2;
  uint pad_1;
  uint pad_2;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  int after;
  uint pad_11;
};

layout(binding = 0, std140) uniform u_block_ubo {
  S_std140 inner[4];
} u;

void a(S a_1[4]) {
}

void b(S s) {
}

void c(mat3x2 m) {
}

void d(vec2 v) {
}

void e(float f_1) {
}

S conv_S(S_std140 val) {
  S tint_symbol = S(val.before, mat3x2(val.m_0, val.m_1, val.m_2), val.after);
  return tint_symbol;
}

S[4] conv_arr_4_S(S_std140 val[4]) {
  S arr[4] = S[4](S(0, mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0), S(0, mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0), S(0, mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0), S(0, mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_S(val[i]);
    }
  }
  return arr;
}

mat3x2 load_u_2_m() {
  return mat3x2(u.inner[2u].m_0, u.inner[2u].m_1, u.inner[2u].m_2);
}

void f() {
  a(conv_arr_4_S(u.inner));
  b(conv_S(u.inner[2u]));
  c(load_u_2_m());
  d(u.inner[0u].m_1.yx);
  e(u.inner[0u].m_1.yx[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
