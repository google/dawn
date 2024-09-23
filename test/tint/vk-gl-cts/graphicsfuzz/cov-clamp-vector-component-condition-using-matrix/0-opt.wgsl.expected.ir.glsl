SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[5];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 1, std140)
uniform tint_symbol_2_1_ubo {
  buf1 tint_symbol_1;
} v_1;
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_2;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  vec4 v = vec4(0.0f);
  int i = 0;
  int x_40 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_43 = v_1.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  int x_46 = v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el;
  int x_49 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  float v_3 = float(x_40);
  float v_4 = float(x_43);
  float v_5 = float(x_46);
  v = vec4(v_3, v_4, v_5, float(x_49));
  int x_53 = v_1.tint_symbol_1.x_GLF_uniform_int_values[4].el;
  i = x_53;
  {
    while(true) {
      int x_58 = i;
      int x_60 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
      if ((x_58 < x_60)) {
      } else {
        break;
      }
      vec4 x_63 = v;
      vec4 x_64 = v;
      vec4 x_65 = v;
      vec4 x_66 = v;
      int x_88 = i;
      float x_92 = v_2.tint_symbol_3.x_GLF_uniform_float_values[0].el;
      vec4 v_6 = vec4(x_63[0u], x_63[1u], x_63[2u], x_63[3u]);
      vec4 v_7 = vec4(x_64[0u], x_64[1u], x_64[2u], x_64[3u]);
      vec4 v_8 = vec4(x_65[0u], x_65[1u], x_65[2u], x_65[3u]);
      if ((mat4(v_6, v_7, v_8, vec4(x_66[0u], x_66[1u], x_66[2u], x_66[3u]))[0u][x_88] > x_92)) {
        int x_96 = i;
        vec4 x_97 = v;
        float x_99 = v_2.tint_symbol_3.x_GLF_uniform_float_values[1].el;
        float x_102 = v_2.tint_symbol_3.x_GLF_uniform_float_values[0].el;
        int x_106 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
        vec4 v_9 = vec4(x_99, x_99, x_99, x_99);
        v[x_96] = clamp(x_97, v_9, vec4(x_102, x_102, x_102, x_102))[x_106];
      }
      {
        int x_109 = i;
        i = (x_109 + 1);
      }
      continue;
    }
  }
  vec4 x_111 = v;
  int x_113 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  float x_114 = float(x_113);
  if (all((x_111 == vec4(x_114, x_114, x_114, x_114)))) {
    int x_122 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_125 = v_1.tint_symbol_1.x_GLF_uniform_int_values[4].el;
    int x_128 = v_1.tint_symbol_1.x_GLF_uniform_int_values[4].el;
    int x_131 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    float v_10 = float(x_122);
    float v_11 = float(x_125);
    float v_12 = float(x_128);
    x_GLF_color = vec4(v_10, v_11, v_12, float(x_131));
  } else {
    int x_135 = v_1.tint_symbol_1.x_GLF_uniform_int_values[4].el;
    float x_136 = float(x_135);
    x_GLF_color = vec4(x_136, x_136, x_136, x_136);
  }
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:85: 'all' : no matching overloaded function found 
ERROR: 0:85: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
