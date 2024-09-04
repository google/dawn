SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[4];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_7;
uniform buf0 x_12;
float func_f1_(inout float b) {
  x_GLF_color = vec4(x_7.x_GLF_uniform_float_values[0].el, x_7.x_GLF_uniform_float_values[0].el, x_7.x_GLF_uniform_float_values[1].el, 1.0f);
  x_GLF_color = x_GLF_color;
  if ((x_7.x_GLF_uniform_float_values[0].el >= b)) {
    float x_104 = x_7.x_GLF_uniform_float_values[0].el;
    return x_104;
  }
  float x_106 = x_7.x_GLF_uniform_float_values[2].el;
  return x_106;
}
void main_1() {
  float a = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  bool x_71 = false;
  bool x_72 = false;
  param = x_7.x_GLF_uniform_float_values[0].el;
  float x_45 = func_f1_(param);
  a = x_45;
  param_1 = (x_7.x_GLF_uniform_float_values[0].el + x_7.x_GLF_uniform_float_values[0].el);
  float x_51 = func_f1_(param_1);
  a = (a + x_51);
  bool x_57 = (a == x_7.x_GLF_uniform_float_values[3].el);
  x_72 = x_57;
  if (x_57) {
    vec4 v = x_GLF_color;
    x_71 = all((v == vec4(x_7.x_GLF_uniform_float_values[0].el, x_7.x_GLF_uniform_float_values[0].el, x_7.x_GLF_uniform_float_values[1].el, x_7.x_GLF_uniform_float_values[0].el)));
    x_72 = x_71;
  }
  if (x_72) {
    float v_1 = float(x_12.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_12.x_GLF_uniform_int_values[1].el);
    float v_3 = float(x_12.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_12.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_12.x_GLF_uniform_int_values[1].el));
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
