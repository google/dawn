SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float x_23 = x_5.x_GLF_uniform_float_values[1].el;
  if ((inversesqrt(x_23) < -1.0f)) {
    float x_30 = x_5.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_30, x_30, x_30, x_30);
  } else {
    float x_33 = x_5.x_GLF_uniform_float_values[1].el;
    float x_35 = x_5.x_GLF_uniform_float_values[0].el;
    float x_37 = x_5.x_GLF_uniform_float_values[0].el;
    float x_39 = x_5.x_GLF_uniform_float_values[1].el;
    x_GLF_color = vec4(x_33, x_35, x_37, x_39);
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
