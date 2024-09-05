#version 310 es


struct mat3x3_f32_std140 {
  vec3 col0;
  vec3 col1;
  vec3 col2;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  mat3x3_f32_std140 tint_symbol[4];
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  mat3 tint_symbol_2[4];
} v_1;
void tint_store_and_preserve_padding_1(inout mat3 target, mat3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
  target[2u] = value_param[2u];
}
void tint_store_and_preserve_padding(inout mat3 target[4], mat3 value_param[4]) {
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
  mat3x3_f32_std140 v_4[4] = v.tint_symbol;
  mat3 v_5[4] = mat3[4](mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      v_5[v_7] = mat3(v_4[v_7].col0, v_4[v_7].col1, v_4[v_7].col2);
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  tint_store_and_preserve_padding(v_1.tint_symbol_2, v_5);
  tint_store_and_preserve_padding_1(v_1.tint_symbol_2[1], mat3(v.tint_symbol[2].col0, v.tint_symbol[2].col1, v.tint_symbol[2].col2));
  v_1.tint_symbol_2[1][0] = v.tint_symbol[0].col1.zxy;
  v_1.tint_symbol_2[1][0][0u] = v.tint_symbol[0].col1.x;
}
