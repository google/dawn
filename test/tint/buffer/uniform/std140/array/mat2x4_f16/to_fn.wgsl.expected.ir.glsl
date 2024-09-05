#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat2x4_f16_std140 {
  f16vec4 col0;
  f16vec4 col1;
};

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  mat2x4_f16_std140 tint_symbol[4];
} v_1;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  float16_t tint_symbol_2;
} v_2;
float16_t a(f16mat2x4 a_1[4]) {
  return a_1[0][0][0u];
}
float16_t b(f16mat2x4 m) {
  return m[0][0u];
}
float16_t c(f16vec4 v) {
  return v[0u];
}
float16_t d(float16_t f) {
  return f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4_f16_std140 v_3[4] = v_1.tint_symbol;
  f16mat2x4 v_4[4] = f16mat2x4[4](f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      v_4[v_6] = f16mat2x4(v_3[v_6].col0, v_3[v_6].col1);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  float16_t v_7 = a(v_4);
  float16_t v_8 = (v_7 + b(f16mat2x4(v_1.tint_symbol[1].col0, v_1.tint_symbol[1].col1)));
  float16_t v_9 = (v_8 + c(v_1.tint_symbol[1].col0.ywxz));
  v_2.tint_symbol_2 = (v_9 + d(v_1.tint_symbol[1].col0.ywxz[0u]));
}
