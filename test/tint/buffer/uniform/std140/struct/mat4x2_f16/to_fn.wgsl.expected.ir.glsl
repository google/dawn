#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S_std140 {
  int before;
  f16vec2 m_col0;
  f16vec2 m_col1;
  f16vec2 m_col2;
  f16vec2 m_col3;
  int after;
};

struct S {
  int before;
  f16mat4x2 m;
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
void c(f16mat4x2 m) {
}
void d(f16vec2 v) {
}
void e(float16_t f) {
}
S tint_convert_S(S_std140 tint_input) {
  return S(tint_input.before, f16mat4x2(tint_input.m_col0, tint_input.m_col1, tint_input.m_col2, tint_input.m_col3), tint_input.after);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S_std140 v_2[4] = v_1.tint_symbol;
  S v_3[4] = S[4](S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
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
  c(f16mat4x2(v_1.tint_symbol[2].m_col0, v_1.tint_symbol[2].m_col1, v_1.tint_symbol[2].m_col2, v_1.tint_symbol[2].m_col3));
  d(v_1.tint_symbol[0].m_col1.yx);
  e(v_1.tint_symbol[0].m_col1.yx[0u]);
}
