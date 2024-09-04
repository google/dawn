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
  float x_30 = x_6.x_GLF_uniform_float_values[1].el;
  a = x_30;
  float x_32 = x_6.x_GLF_uniform_float_values[3].el;
  float x_34 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_32 > x_34)) {
    float x_39 = x_6.x_GLF_uniform_float_values[0].el;
    float x_40 = a;
    a = (x_40 + x_39);
    float x_42 = a;
    x_GLF_color = vec4(x_42, x_42, x_42, x_42);
    float x_45 = x_6.x_GLF_uniform_float_values[3].el;
    float x_47 = x_6.x_GLF_uniform_float_values[1].el;
    if ((x_45 > x_47)) {
      float x_52 = x_GLF_color.x;
      float x_53 = a;
      a = (x_53 + x_52);
      float x_56 = x_6.x_GLF_uniform_float_values[2].el;
      x_GLF_color = vec4(x_56, x_56, x_56, x_56);
    }
  }
  float x_58 = a;
  x_GLF_color = vec4(x_58, 0.0f, 0.0f, 1.0f);
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
