#version 310 es


struct S_std140 {
  int before;
  vec2 m_col0;
  vec2 m_col1;
  int after;
};

struct S {
  int before;
  mat2 m;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  S_std140 tint_symbol[4];
} v;
shared S w[4];
S tint_convert_S(S_std140 tint_input) {
  return S(tint_input.before, mat2(tint_input.m_col0, tint_input.m_col1), tint_input.after);
}
void f_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      w[v_2] = S(0, mat2(vec2(0.0f), vec2(0.0f)), 0);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  barrier();
  S_std140 v_3[4] = v.tint_symbol;
  S v_4[4] = S[4](S(0, mat2(vec2(0.0f), vec2(0.0f)), 0), S(0, mat2(vec2(0.0f), vec2(0.0f)), 0), S(0, mat2(vec2(0.0f), vec2(0.0f)), 0), S(0, mat2(vec2(0.0f), vec2(0.0f)), 0));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      v_4[v_6] = tint_convert_S(v_3[v_6]);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  w = v_4;
  w[1] = tint_convert_S(v.tint_symbol[2]);
  w[3].m = mat2(v.tint_symbol[2].m_col0, v.tint_symbol[2].m_col1);
  w[1].m[0] = v.tint_symbol[0].m_col1.yx;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
