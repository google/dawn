#version 310 es

struct S {
  int before;
  uint pad;
  mat3x2 m;
  uint pad_1;
  uint pad_2;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  uint pad_7;
  uint pad_8;
  int after;
  uint pad_9;
  uint pad_10;
  uint pad_11;
  uint pad_12;
  uint pad_13;
  uint pad_14;
  uint pad_15;
  uint pad_16;
  uint pad_17;
  uint pad_18;
  uint pad_19;
  uint pad_20;
  uint pad_21;
  uint pad_22;
  uint pad_23;
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
  int after;
  uint pad_9;
  uint pad_10;
  uint pad_11;
  uint pad_12;
  uint pad_13;
  uint pad_14;
  uint pad_15;
  uint pad_16;
  uint pad_17;
  uint pad_18;
  uint pad_19;
  uint pad_20;
  uint pad_21;
  uint pad_22;
  uint pad_23;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  S_std140 inner[4];
} u;

shared S w[4];
S conv_S(S_std140 val) {
  return S(val.before, val.pad, mat3x2(val.m_0, val.m_1, val.m_2), val.pad_1, val.pad_2, val.pad_3, val.pad_4, val.pad_5, val.pad_6, val.pad_7, val.pad_8, val.after, val.pad_9, val.pad_10, val.pad_11, val.pad_12, val.pad_13, val.pad_14, val.pad_15, val.pad_16, val.pad_17, val.pad_18, val.pad_19, val.pad_20, val.pad_21, val.pad_22, val.pad_23);
}

S[4] conv_arr4_S(S_std140 val[4]) {
  S arr[4] = S[4](S(0, 0u, mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, mat3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_S(val[i]);
    }
  }
  return arr;
}

mat3x2 load_u_inner_2_m() {
  return mat3x2(u.inner[2u].m_0, u.inner[2u].m_1, u.inner[2u].m_2);
}

void f(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      S tint_symbol = S(0, 0u, mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
      w[i] = tint_symbol;
    }
  }
  barrier();
  w = conv_arr4_S(u.inner);
  w[1] = conv_S(u.inner[2u]);
  w[3].m = load_u_inner_2_m();
  w[1].m[0] = u.inner[0u].m_1.yx;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}
