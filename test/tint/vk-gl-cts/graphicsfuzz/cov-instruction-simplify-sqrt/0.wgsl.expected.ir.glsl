SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
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
  float x_31 = x_5.x_GLF_uniform_float_values[0].el;
  if ((sqrt(x_31) < -1.0f)) {
    int x_10 = x_7.x_GLF_uniform_int_values[1].el;
    float x_38 = float(x_10);
    x_GLF_color = vec4(x_38, x_38, x_38, x_38);
  } else {
    int x_11 = x_7.x_GLF_uniform_int_values[0].el;
    float x_41 = float(x_11);
    int x_12 = x_7.x_GLF_uniform_int_values[1].el;
    float x_43 = float(x_12);
    x_GLF_color = vec4(x_41, x_43, x_43, x_41);
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
