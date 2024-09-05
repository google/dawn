#version 310 es


struct mat2x2_f32_std140 {
  vec2 col0;
  vec2 col1;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  mat2x2_f32_std140 tint_symbol[4];
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  float tint_symbol_2;
} v_1;
int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v_2 = i();
  mat2 v_3 = mat2(v.tint_symbol[v_2].col0, v.tint_symbol[v_2].col1);
  vec2 v_4 = v_3[i()];
  mat2x2_f32_std140 v_5[4] = v.tint_symbol;
  mat2 v_6[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      v_6[v_8] = mat2(v_5[v_8].col0, v_5[v_8].col1);
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  mat2 l_a[4] = v_6;
  mat2 l_a_i = v_3;
  vec2 l_a_i_i = v_4;
  v_1.tint_symbol_2 = (((v_4[0u] + l_a[0][0][0u]) + l_a_i[0][0u]) + l_a_i_i[0u]);
}
