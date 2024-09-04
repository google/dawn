SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float a = 0.0f;
  a = x_6.x_GLF_uniform_float_values[1].el;
  if ((x_6.x_GLF_uniform_float_values[3].el > x_6.x_GLF_uniform_float_values[0].el)) {
    a = (a + x_6.x_GLF_uniform_float_values[0].el);
    x_GLF_color = vec4(a);
    if ((x_6.x_GLF_uniform_float_values[3].el > x_6.x_GLF_uniform_float_values[1].el)) {
      a = (a + x_GLF_color.x);
      x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[2].el);
    }
  }
  x_GLF_color = vec4(a, 0.0f, 0.0f, 1.0f);
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
