SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_14;
float f1_f1_(inout float a) {
  int b = 0;
  float c = 0.0f;
  b = 8;
  float x_71 = tint_symbol.y;
  float x_73 = x_9.x_GLF_uniform_float_values[0].el;
  if ((x_71 >= x_73)) {
    int x_77 = b;
    b = (x_77 + 1);
    int x_79 = b;
    b = (x_79 + 1);
  }
  float x_81 = a;
  float x_83 = x_9.x_GLF_uniform_float_values[1].el;
  if ((x_81 < x_83)) {
    float x_88 = x_9.x_GLF_uniform_float_values[1].el;
    return x_88;
  }
  int x_89 = b;
  c = float(min(max(x_89, 0), 2));
  float x_92 = c;
  return x_92;
}
void main_1() {
  float a_1 = 0.0f;
  float param = 0.0f;
  float x_43 = x_9.x_GLF_uniform_float_values[1].el;
  param = x_43;
  float x_44 = f1_f1_(param);
  a_1 = x_44;
  float x_45 = a_1;
  float x_47 = x_9.x_GLF_uniform_float_values[2].el;
  if ((x_45 == x_47)) {
    int x_53 = x_14.x_GLF_uniform_int_values[1].el;
    int x_56 = x_14.x_GLF_uniform_int_values[0].el;
    int x_59 = x_14.x_GLF_uniform_int_values[0].el;
    int x_62 = x_14.x_GLF_uniform_int_values[1].el;
    float v = float(x_53);
    float v_1 = float(x_56);
    float v_2 = float(x_59);
    x_GLF_color = vec4(v, v_1, v_2, float(x_62));
  } else {
    int x_66 = x_14.x_GLF_uniform_int_values[0].el;
    float x_67 = float(x_66);
    x_GLF_color = vec4(x_67, x_67, x_67, x_67);
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
