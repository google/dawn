#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S_std140 {
  int before;
  f16vec2 m_col0;
  f16vec2 m_col1;
  f16vec2 m_col2;
  int after;
};

struct S {
  int before;
  f16mat3x2 m;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  S_std140 tint_symbol[4];
} v;
S p[4] = S[4](S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
S tint_convert_S(S_std140 tint_input) {
  return S(tint_input.before, f16mat3x2(tint_input.m_col0, tint_input.m_col1, tint_input.m_col2), tint_input.after);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S_std140 v_1[4] = v.tint_symbol;
  S v_2[4] = S[4](S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      v_2[v_4] = tint_convert_S(v_1[v_4]);
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  p = v_2;
  p[1] = tint_convert_S(v.tint_symbol[2]);
  p[3].m = f16mat3x2(v.tint_symbol[2].m_col0, v.tint_symbol[2].m_col1, v.tint_symbol[2].m_col2);
  p[1].m[0] = v.tint_symbol[0].m_col1.yx;
}
