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
  bool x_139 = false;
  bool x_125 = (x < 1);
  x_139 = x_125;
  if (!(x_125)) {
    bool x_130 = (x > 1);
    x_138 = x_130;
    if (x_130) {
      x_137 = (v.tint_symbol_3.one > v_1.tint_symbol_5.x_GLF_uniform_float_values[0].el);
      x_138 = x_137;
    }
    x_139 = x_138;
  }
  if (x_139) {
    return;
  }
  if ((v.tint_symbol_3.one == v_1.tint_symbol_5.x_GLF_uniform_float_values[0].el)) {
    i = 0;
    {
      while(true) {
        if ((i < 2)) {
        } else {
          break;
        }
        {
          int x_155 = min(max(x, 0), 3);
          int x_156 = i;
          m[x_155][x_156] = (m[x_155][i] + v_1.tint_symbol_5.x_GLF_uniform_float_values[0].el);
          i = (i + 1);
        }
        continue;
      }
    }
  }
}
void func1_() {
  int param = 0;
  if ((tint_symbol.y < 0.0f)) {
    return;
  }
  param = 1;
  func0_i1_(param);
}
void main_1() {
  m = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
  func1_();
  func1_();
  float v_3 = float(v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el);
  vec2 v_4 = vec2(v_3, float(v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el));
  float v_5 = float(v_2.tint_symbol_7.x_GLF_uniform_int_values[1].el);
  vec2 v_6 = vec2(v_5, float(v_2.tint_symbol_7.x_GLF_uniform_int_values[1].el));
  float v_7 = float(v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el);
  vec2 v_8 = vec2(v_7, float(v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el));
  float v_9 = float(v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el);
  mat4x2 x_83 = mat4x2(v_4, v_6, v_8, vec2(v_9, float(v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el)));
  bool v_10 = all((m[0u] == x_83[0u]));
  bool v_11 = (v_10 & all((m[1u] == x_83[1u])));
  bool v_12 = (v_11 & all((m[2u] == x_83[2u])));
  if ((v_12 & all((m[3u] == x_83[3u])))) {
    float v_13 = float(v_2.tint_symbol_7.x_GLF_uniform_int_values[3].el);
    float v_14 = float(v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el);
    float v_15 = float(v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_13, v_14, v_15, float(v_2.tint_symbol_7.x_GLF_uniform_int_values[3].el));
  } else {
    x_GLF_color = vec4(float(v_2.tint_symbol_7.x_GLF_uniform_int_values[0].el));
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
ERROR: 0:104: 'all' : no matching overloaded function found 
ERROR: 0:104: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:104: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
