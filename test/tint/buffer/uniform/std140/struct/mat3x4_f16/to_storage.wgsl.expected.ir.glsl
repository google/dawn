#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S_std140 {
  int before;
  f16vec4 m_col0;
  f16vec4 m_col1;
  f16vec4 m_col2;
  int after;
};

struct S {
  int before;
  f16mat3x4 m;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  S_std140 tint_symbol[4];
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  S tint_symbol_2[4];
} v_1;
S tint_convert_S(S_std140 tint_input) {
  return S(tint_input.before, f16mat3x4(tint_input.m_col0, tint_input.m_col1, tint_input.m_col2), tint_input.after);
}
void tint_store_and_preserve_padding_1(inout S target, S value_param) {
  target.before = value_param.before;
  target.m = value_param.m;
  target.after = value_param.after;
}
void tint_store_and_preserve_padding(inout S target[4], S value_param[4]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(target[v_3], value_param[v_3]);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S_std140 v_4[4] = v.tint_symbol;
  S v_5[4] = S[4](S(0, f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0));
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      v_5[v_7] = tint_convert_S(v_4[v_7]);
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  tint_store_and_preserve_padding(v_1.tint_symbol_2, v_5);
  tint_store_and_preserve_padding_1(v_1.tint_symbol_2[1], tint_convert_S(v.tint_symbol[2]));
  v_1.tint_symbol_2[3].m = f16mat3x4(v.tint_symbol[2].m_col0, v.tint_symbol[2].m_col1, v.tint_symbol[2].m_col2);
  v_1.tint_symbol_2[1].m[0] = v.tint_symbol[0].m_col1.ywxz;
}
