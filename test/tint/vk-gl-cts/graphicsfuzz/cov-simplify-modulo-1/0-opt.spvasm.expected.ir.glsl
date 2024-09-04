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
  if ((a == x_6.x_GLF_uniform_float_values[0].el)) {
    float v = float(x_8.x_GLF_uniform_int_values[0].el);
    float v_1 = a;
    float v_2 = a;
    x_GLF_color = vec4(v, v_1, v_2, float(x_8.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(a);
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
