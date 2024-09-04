SKIP: FAILED

#version 310 es

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
precision highp float;
precision highp int;


mat4x2 m = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
uniform buf2 x_10;
uniform buf0 x_12;
vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_16;
vec4 x_GLF_color = vec4(0.0f);
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
      float x_134 = x_10.one;
      float x_136 = x_12.x_GLF_uniform_float_values[0].el;
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
  float x_143 = x_10.one;
  float x_145 = x_12.x_GLF_uniform_float_values[0].el;
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
          float x_158 = x_12.x_GLF_uniform_float_values[0].el;
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
  int x_56 = x_16.x_GLF_uniform_int_values[0].el;
  int x_59 = x_16.x_GLF_uniform_int_values[0].el;
  int x_62 = x_16.x_GLF_uniform_int_values[1].el;
  int x_65 = x_16.x_GLF_uniform_int_values[1].el;
  int x_68 = x_16.x_GLF_uniform_int_values[0].el;
  int x_71 = x_16.x_GLF_uniform_int_values[0].el;
  int x_74 = x_16.x_GLF_uniform_int_values[0].el;
  int x_77 = x_16.x_GLF_uniform_int_values[0].el;
  float v = float(x_56);
  vec2 v_1 = vec2(v, float(x_59));
  float v_2 = float(x_62);
  vec2 v_3 = vec2(v_2, float(x_65));
  float v_4 = float(x_68);
  vec2 v_5 = vec2(v_4, float(x_71));
  float v_6 = float(x_74);
  mat4x2 x_83 = mat4x2(v_1, v_3, v_5, vec2(v_6, float(x_77)));
  bool v_7 = all((x_54[0u] == x_83[0u]));
  bool v_8 = (v_7 & all((x_54[1u] == x_83[1u])));
  bool v_9 = (v_8 & all((x_54[2u] == x_83[2u])));
  if ((v_9 & all((x_54[3u] == x_83[3u])))) {
    int x_107 = x_16.x_GLF_uniform_int_values[3].el;
    int x_110 = x_16.x_GLF_uniform_int_values[0].el;
    int x_113 = x_16.x_GLF_uniform_int_values[0].el;
    int x_116 = x_16.x_GLF_uniform_int_values[3].el;
    float v_10 = float(x_107);
    float v_11 = float(x_110);
    float v_12 = float(x_113);
    x_GLF_color = vec4(v_10, v_11, v_12, float(x_116));
  } else {
    int x_120 = x_16.x_GLF_uniform_int_values[0].el;
    float x_121 = float(x_120);
    x_GLF_color = vec4(x_121, x_121, x_121, x_121);
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
