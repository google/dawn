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
  float f = 0.0f;
  bool x_48 = false;
  bool x_49 = false;
  f = 1626.509033203125f;
  bool x_41 = (x_6.x_GLF_uniform_int_values[0].el == (x_6.x_GLF_uniform_int_values[0].el + x_6.x_GLF_uniform_int_values[1].el));
  x_49 = x_41;
  if (!(x_41)) {
    x_48 = (f > x_8.x_GLF_uniform_float_values[0].el);
    x_49 = x_48;
  }
  if (x_49) {
    float v = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_6.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[1].el));
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
