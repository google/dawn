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
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_7;
void main_1() {
  float v = ldexp(x_5.x_GLF_uniform_float_values[0].el, 100);
  if ((v == x_5.x_GLF_uniform_float_values[0].el)) {
    float v_1 = float(x_7.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_7.x_GLF_uniform_int_values[0].el);
    float v_3 = float(x_7.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_7.x_GLF_uniform_int_values[1].el));
  } else {
    float v_4 = float(x_7.x_GLF_uniform_int_values[1].el);
    float v_5 = float(x_7.x_GLF_uniform_int_values[0].el);
    float v_6 = float(x_7.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_4, v_5, v_6, float(x_7.x_GLF_uniform_int_values[1].el));
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
