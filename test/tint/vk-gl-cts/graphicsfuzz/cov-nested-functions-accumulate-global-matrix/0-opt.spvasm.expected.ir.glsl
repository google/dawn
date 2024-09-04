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
  bool x_139 = false;
  bool x_125 = (x < 1);
  x_139 = x_125;
  if (!(x_125)) {
    bool x_130 = (x > 1);
    x_138 = x_130;
    if (x_130) {
      x_137 = (x_10.one > x_12.x_GLF_uniform_float_values[0].el);
      x_138 = x_137;
    }
    x_139 = x_138;
  }
  if (x_139) {
    return;
  }
  if ((x_10.one == x_12.x_GLF_uniform_float_values[0].el)) {
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
          m[x_155][x_156] = (m[x_155][i] + x_12.x_GLF_uniform_float_values[0].el);
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
  float v = float(x_16.x_GLF_uniform_int_values[0].el);
  vec2 v_1 = vec2(v, float(x_16.x_GLF_uniform_int_values[0].el));
  float v_2 = float(x_16.x_GLF_uniform_int_values[1].el);
  vec2 v_3 = vec2(v_2, float(x_16.x_GLF_uniform_int_values[1].el));
  float v_4 = float(x_16.x_GLF_uniform_int_values[0].el);
  vec2 v_5 = vec2(v_4, float(x_16.x_GLF_uniform_int_values[0].el));
  float v_6 = float(x_16.x_GLF_uniform_int_values[0].el);
  mat4x2 x_83 = mat4x2(v_1, v_3, v_5, vec2(v_6, float(x_16.x_GLF_uniform_int_values[0].el)));
  bool v_7 = all((m[0u] == x_83[0u]));
  bool v_8 = (v_7 & all((m[1u] == x_83[1u])));
  bool v_9 = (v_8 & all((m[2u] == x_83[2u])));
  if ((v_9 & all((m[3u] == x_83[3u])))) {
    float v_10 = float(x_16.x_GLF_uniform_int_values[3].el);
    float v_11 = float(x_16.x_GLF_uniform_int_values[0].el);
    float v_12 = float(x_16.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_10, v_11, v_12, float(x_16.x_GLF_uniform_int_values[3].el));
  } else {
    x_GLF_color = vec4(float(x_16.x_GLF_uniform_int_values[0].el));
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
