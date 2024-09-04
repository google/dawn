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
  if ((inversesqrt(x_5.x_GLF_uniform_float_values[1].el) < -1.0f)) {
    x_GLF_color = vec4(x_5.x_GLF_uniform_float_values[0].el);
  } else {
    x_GLF_color = vec4(x_5.x_GLF_uniform_float_values[1].el, x_5.x_GLF_uniform_float_values[0].el, x_5.x_GLF_uniform_float_values[0].el, x_5.x_GLF_uniform_float_values[1].el);
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
