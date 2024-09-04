SKIP: FAILED

#version 310 es

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
  strided_arr_1 x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_8;
uniform buf1 x_11;
vec4 x_GLF_color = vec4(0.0f);
int f1_() {
  int a = 0;
  int i = 0;
  a = 256;
  float x_65 = tint_symbol.y;
  float x_67 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_65 > x_67)) {
    int x_71 = a;
    a = (x_71 + 1);
  }
  int x_73 = a;
  i = bitCount(x_73);
  int x_75 = i;
  int x_77 = x_11.x_GLF_uniform_int_values[0].el;
  if ((x_75 < x_77)) {
    int x_82 = x_11.x_GLF_uniform_int_values[0].el;
    return x_82;
  }
  int x_83 = i;
  return x_83;
}
void main_1() {
  int a_1 = 0;
  int x_38 = f1_();
  a_1 = x_38;
  int x_39 = a_1;
  int x_41 = x_11.x_GLF_uniform_int_values[2].el;
  if ((x_39 == x_41)) {
    int x_47 = x_11.x_GLF_uniform_int_values[0].el;
    int x_50 = x_11.x_GLF_uniform_int_values[1].el;
    int x_53 = x_11.x_GLF_uniform_int_values[1].el;
    int x_56 = x_11.x_GLF_uniform_int_values[0].el;
    float v = float(x_47);
    float v_1 = float(x_50);
    float v_2 = float(x_53);
    x_GLF_color = vec4(v, v_1, v_2, float(x_56));
  } else {
    int x_60 = x_11.x_GLF_uniform_int_values[1].el;
    float x_61 = float(x_60);
    x_GLF_color = vec4(x_61, x_61, x_61, x_61);
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
