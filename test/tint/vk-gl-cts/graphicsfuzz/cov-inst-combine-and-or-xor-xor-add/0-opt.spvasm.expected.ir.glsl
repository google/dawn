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
  data[x_33] = x_8.x_GLF_uniform_float_values[0].el;
  int x_38 = x_6.x_GLF_uniform_int_values[1].el;
  data[x_38] = x_8.x_GLF_uniform_float_values[1].el;
  a = data[(1 ^ (x_6.x_GLF_uniform_int_values[1].el & 2))];
  if ((a == x_8.x_GLF_uniform_float_values[1].el)) {
    x_GLF_color = vec4(x_8.x_GLF_uniform_float_values[1].el, x_8.x_GLF_uniform_float_values[0].el, x_8.x_GLF_uniform_float_values[0].el, x_8.x_GLF_uniform_float_values[1].el);
  } else {
    x_GLF_color = vec4(x_8.x_GLF_uniform_float_values[0].el);
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
