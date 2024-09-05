#version 310 es


struct S_std140 {
  int before;
  vec3 m_col0;
  vec3 m_col1;
  vec3 m_col2;
  int after;
};

struct S {
  int before;
  mat3 m;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  S_std140 tint_symbol[4];
} v_1;
void a(S a_1[4]) {
}
void b(S s) {
}
void c(mat3 m) {
}
void d(vec3 v) {
}
void e(float f) {
}
S tint_convert_S(S_std140 tint_input) {
  return S(tint_input.before, mat3(tint_input.m_col0, tint_input.m_col1, tint_input.m_col2), tint_input.after);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S_std140 v_2[4] = v_1.tint_symbol;
  S v_3[4] = S[4](S(0, mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      v_3[v_5] = tint_convert_S(v_2[v_5]);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  a(v_3);
  b(tint_convert_S(v_1.tint_symbol[2]));
  c(mat3(v_1.tint_symbol[2].m_col0, v_1.tint_symbol[2].m_col1, v_1.tint_symbol[2].m_col2));
  d(v_1.tint_symbol[0].m_col1.zxy);
  e(v_1.tint_symbol[0].m_col1.zxy[0u]);
}
