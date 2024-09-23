SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 1, std140)
uniform tint_symbol_2_1_ubo {
  buf1 tint_symbol_1;
} v;
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_1;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
int tint_mod_i32(int lhs, int rhs) {
  int v_2 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_2) * v_2));
}
void main_1() {
  mat4 m0 = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  int c = 0;
  mat4 m1 = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  int x_40 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  float x_41 = float(x_40);
  vec4 v_3 = vec4(x_41, 0.0f, 0.0f, 0.0f);
  vec4 v_4 = vec4(0.0f, x_41, 0.0f, 0.0f);
  vec4 v_5 = vec4(0.0f, 0.0f, x_41, 0.0f);
  m0 = mat4(v_3, v_4, v_5, vec4(0.0f, 0.0f, 0.0f, x_41));
  int x_48 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  c = x_48;
  {
    while(true) {
      int x_53 = c;
      int x_55 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
      if ((x_53 < x_55)) {
      } else {
        break;
      }
      mat4 x_58 = m0;
      m1 = x_58;
      int x_59 = c;
      int x_61 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
      int x_64 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
      float x_66 = v_1.tint_symbol_3.x_GLF_uniform_float_values[0].el;
      m1[tint_mod_i32(x_59, x_61)][x_64] = x_66;
      int x_68 = c;
      int x_70 = v.tint_symbol_1.x_GLF_uniform_int_values[3].el;
      int x_73 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
      float x_75 = v_1.tint_symbol_3.x_GLF_uniform_float_values[0].el;
      m0[tint_mod_i32(x_68, x_70)][x_73] = x_75;
      {
        int x_77 = c;
        c = (x_77 + 1);
      }
      continue;
    }
  }
  mat4 x_79 = m0;
  int x_81 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_84 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  int x_87 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_90 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_93 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_96 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  int x_99 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_102 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_105 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_108 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  int x_111 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_114 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_117 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_120 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  int x_123 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_126 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  float v_6 = float(x_81);
  float v_7 = float(x_84);
  float v_8 = float(x_87);
  vec4 v_9 = vec4(v_6, v_7, v_8, float(x_90));
  float v_10 = float(x_93);
  float v_11 = float(x_96);
  float v_12 = float(x_99);
  vec4 v_13 = vec4(v_10, v_11, v_12, float(x_102));
  float v_14 = float(x_105);
  float v_15 = float(x_108);
  float v_16 = float(x_111);
  vec4 v_17 = vec4(v_14, v_15, v_16, float(x_114));
  float v_18 = float(x_117);
  float v_19 = float(x_120);
  float v_20 = float(x_123);
  mat4 x_132 = mat4(v_9, v_13, v_17, vec4(v_18, v_19, v_20, float(x_126)));
  bool v_21 = all((x_79[0u] == x_132[0u]));
  bool v_22 = (v_21 & all((x_79[1u] == x_132[1u])));
  bool v_23 = (v_22 & all((x_79[2u] == x_132[2u])));
  if ((v_23 & all((x_79[3u] == x_132[3u])))) {
    int x_156 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    int x_159 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_162 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_165 = v.tint_symbol_1.x_GLF_uniform_int_values[2].el;
    float v_24 = float(x_156);
    float v_25 = float(x_159);
    float v_26 = float(x_162);
    x_GLF_color = vec4(v_24, v_25, v_26, float(x_165));
  } else {
    int x_169 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    float x_170 = float(x_169);
    x_GLF_color = vec4(x_170, x_170, x_170, x_170);
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
ERROR: 0:37: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:37: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:37: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
