SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf2 {
  float one;
};

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[1];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};

mat4x2 m = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
layout(binding = 2, std140)
uniform tint_symbol_4_1_ubo {
  buf2 tint_symbol_3;
} v;
layout(binding = 0, std140)
uniform tint_symbol_6_1_ubo {
  buf0 tint_symbol_5;
} v_1;
vec4 tint_symbol = vec4(0.0f);
layout(binding = 1, std140)
uniform tint_symbol_8_1_ubo {
  buf1 tint_symbol_7;
} v_2;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
void func0_i1_(inout int x) {
  int i = 0;
  bool x_137 = false;
  bool x_138 = false;
  bool x_138_phi = false;
  bool x_139_phi = false;
  int x_124 = x;
  bool x_125 = (x_124 < 1);
  x_139_phi = x_125;
  if (!(x_125)) {
    int x_129 = x;
    bool x_130 = (x_129 > 1);
    x_138_phi = x_130;
    if (x_130) {
      float x_134 = v.tint_symbol_3.one;
      float x_136 = v_1.tint_symbol_5.x_GLF_uniform_float_values[0].el;
      x_137 = (x_134 > x_136);
      x_138_phi = x_137;
    }
    x_138 = x_138_phi;
    x_139_phi = x_138;
  }
  bool x_139 = x_139_phi;
  if (x_139) {
    return;
  }
  float x_143 = v.tint_symbol_3.one;
  float x_145 = v_1.tint_symbol_5.x_GLF_uniform_float_values[0].el;
  if ((x_143 == x_145)) {
    i = 0;
    {
      while(true) {
        int x_150 = i;
        if ((x_150 < 2)) {
        } else {
          break;
        }
        {
          int x_154 = x;
          int x_155 = min(max(x_154, 0), 3);
          int x_156 = i;
          float x_158 = v_1.tint_symbol_5.x_GLF_uniform_float_values[0].el;
          float x_160 = m[x_155][x_156];
          m[x_155][x_156] = (x_160 + x_158);
          int x_163 = i;
          i = (x_163 + 1);
        }
        continue;
      }
    }
  }
}
void func1_() {
  int param = 0;
  float x_167 = tint_symbol.y;
  if ((x_167 < 0.0f)) {
    return;
  }
  param = 1;
  func0_i1_(param);
}
void main_1() {
  m = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
  func1_();
  func1_();
  mat4x2 x_54 = m;
  int x_56 = v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el;
  int x_59 = v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el;
  int x_62 = v_2.tint_symbol_7.x_GLF_uniform_int_values[1].el;
  int x_65 = v_2.tint_symbol_7.x_GLF_uniform_int_values[1].el;
  int x_68 = v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el;
  int x_71 = v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el;
  int x_74 = v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el;
  int x_77 = v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el;
  float v_3 = float(x_56);
  vec2 v_4 = vec2(v_3, float(x_59));
  float v_5 = float(x_62);
  vec2 v_6 = vec2(v_5, float(x_65));
  float v_7 = float(x_68);
  vec2 v_8 = vec2(v_7, float(x_71));
  float v_9 = float(x_74);
  mat4x2 x_83 = mat4x2(v_4, v_6, v_8, vec2(v_9, float(x_77)));
  bool v_10 = all((x_54[0u] == x_83[0u]));
  bool v_11 = (v_10 & all((x_54[1u] == x_83[1u])));
  bool v_12 = (v_11 & all((x_54[2u] == x_83[2u])));
  if ((v_12 & all((x_54[3u] == x_83[3u])))) {
    int x_107 = v_2.tint_symbol_7.x_GLF_uniform_int_values[3].el;
    int x_110 = v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el;
    int x_113 = v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el;
    int x_116 = v_2.tint_symbol_7.x_GLF_uniform_int_values[3].el;
    float v_13 = float(x_107);
    float v_14 = float(x_110);
    float v_15 = float(x_113);
    x_GLF_color = vec4(v_13, v_14, v_15, float(x_116));
  } else {
    int x_120 = v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el;
    float x_121 = float(x_120);
    x_GLF_color = vec4(x_121, x_121, x_121, x_121);
  }
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:128: 'all' : no matching overloaded function found 
ERROR: 0:128: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:128: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
