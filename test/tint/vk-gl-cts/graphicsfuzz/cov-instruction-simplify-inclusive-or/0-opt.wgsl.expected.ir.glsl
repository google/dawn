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
  strided_arr_1 x_GLF_uniform_float_values[1];
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
  float a = 0.0f;
  int x_31 = x_6.x_GLF_uniform_int_values[0].el;
  int x_34 = x_6.x_GLF_uniform_int_values[1].el;
  int x_38 = x_6.x_GLF_uniform_int_values[0].el;
  float v = float(x_31);
  a = vec2(v, float(x_34))[(x_38 | 1)];
  float x_41 = a;
  float x_43 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_41 == x_43)) {
    float x_48 = a;
    int x_50 = x_6.x_GLF_uniform_int_values[0].el;
    int x_53 = x_6.x_GLF_uniform_int_values[0].el;
    float x_55 = a;
    float v_1 = float(x_50);
    x_GLF_color = vec4(x_48, v_1, float(x_53), x_55);
  } else {
    float x_57 = a;
    x_GLF_color = vec4(x_57, x_57, x_57, x_57);
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
