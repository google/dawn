#version 310 es


struct mat4x3_f32_std140 {
  vec3 col0;
  vec3 col1;
  vec3 col2;
  vec3 col3;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  mat4x3_f32_std140 tint_symbol[4];
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
  mat4x3 v_3 = mat4x3(v.tint_symbol[v_2].col0, v.tint_symbol[v_2].col1, v.tint_symbol[v_2].col2, v.tint_symbol[v_2].col3);
  vec3 v_4 = v_3[i()];
  mat4x3_f32_std140 v_5[4] = v.tint_symbol;
  mat4x3 v_6[4] = mat4x3[4](mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      v_6[v_8] = mat4x3(v_5[v_8].col0, v_5[v_8].col1, v_5[v_8].col2, v_5[v_8].col3);
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  mat4x3 l_a[4] = v_6;
  mat4x3 l_a_i = v_3;
  vec3 l_a_i_i = v_4;
  v_1.tint_symbol_2 = (((v_4[0u] + l_a[0][0][0u]) + l_a_i[0][0u]) + l_a_i_i[0u]);
}
