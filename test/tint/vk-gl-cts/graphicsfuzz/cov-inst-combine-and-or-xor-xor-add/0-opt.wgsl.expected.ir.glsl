SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct strided_arr_1 {
  float el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float data[2] = float[2](0.0f, 0.0f);
  float a = 0.0f;
  int x_33 = x_6.x_GLF_uniform_int_values[0].el;
  float x_35 = x_8.x_GLF_uniform_float_values[0].el;
  data[x_33] = x_35;
  int x_38 = x_6.x_GLF_uniform_int_values[1].el;
  float x_40 = x_8.x_GLF_uniform_float_values[1].el;
  data[x_38] = x_40;
  int x_43 = x_6.x_GLF_uniform_int_values[1].el;
  float x_47 = data[(1 ^ (x_43 & 2))];
  a = x_47;
  float x_48 = a;
  float x_50 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_48 == x_50)) {
    float x_56 = x_8.x_GLF_uniform_float_values[1].el;
    float x_58 = x_8.x_GLF_uniform_float_values[0].el;
    float x_60 = x_8.x_GLF_uniform_float_values[0].el;
    float x_62 = x_8.x_GLF_uniform_float_values[1].el;
    x_GLF_color = vec4(x_56, x_58, x_60, x_62);
  } else {
    float x_65 = x_8.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_65, x_65, x_65, x_65);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
