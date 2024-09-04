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
  float v = float(x_6.x_GLF_uniform_int_values[0].el);
  vec2 v_1 = vec2(v, float(x_6.x_GLF_uniform_int_values[1].el));
  a = v_1[(x_6.x_GLF_uniform_int_values[0].el | 1)];
  if ((a == x_8.x_GLF_uniform_float_values[0].el)) {
    float v_2 = a;
    float v_3 = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_4 = float(x_6.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_2, v_3, v_4, a);
  } else {
    x_GLF_color = vec4(a);
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
