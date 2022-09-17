#version 310 es

struct S {
  int before;
  uint pad;
  mat4x2 m;
  int after;
  uint pad_1;
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

layout(binding = 0, std140) uniform u_block_std140_ubo {
  S_std140 inner[4];
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  S inner[4];
} s;

S conv_S(S_std140 val) {
  return S(val.before, val.pad, mat4x2(val.m_0, val.m_1, val.m_2, val.m_3), val.after, val.pad_1);
}

S[4] conv_arr4_S(S_std140 val[4]) {
  S arr[4] = S[4](S(0, 0u, mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0, 0u), S(0, 0u, mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0, 0u), S(0, 0u, mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0, 0u), S(0, 0u, mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0, 0u));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_S(val[i]);
    }
  }
  return arr;
}

mat4x2 load_u_inner_2_m() {
  return mat4x2(u.inner[2u].m_0, u.inner[2u].m_1, u.inner[2u].m_2, u.inner[2u].m_3);
}

void f() {
  s.inner = conv_arr4_S(u.inner);
  s.inner[1] = conv_S(u.inner[2u]);
  s.inner[3].m = load_u_inner_2_m();
  s.inner[1].m[0] = u.inner[0u].m_1.yx;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
