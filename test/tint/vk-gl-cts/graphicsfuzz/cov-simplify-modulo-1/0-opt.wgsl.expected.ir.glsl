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
  strided_arr_1 x_GLF_uniform_int_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_8;
void main_1() {
  float a = 0.0f;
  float x_30 = x_6.x_GLF_uniform_float_values[0].el;
  a = (x_30 - (1.0f * floor((x_30 / 1.0f))));
  float x_32 = a;
  float x_34 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_32 == x_34)) {
    int x_40 = x_8.x_GLF_uniform_int_values[0].el;
    float x_42 = a;
    float x_43 = a;
    int x_45 = x_8.x_GLF_uniform_int_values[0].el;
    float v = float(x_40);
    x_GLF_color = vec4(v, x_42, x_43, float(x_45));
  } else {
    float x_48 = a;
    x_GLF_color = vec4(x_48, x_48, x_48, x_48);
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
