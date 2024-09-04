SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 v = vec4(0.0f);
  float f = 0.0f;
  bool x_56 = false;
  bool x_57 = false;
  float v_1 = float(x_6.x_GLF_uniform_int_values[0].el);
  float v_2 = float(x_6.x_GLF_uniform_int_values[0].el);
  v = vec4(v_1, v_2, -621.59600830078125f, float(x_6.x_GLF_uniform_int_values[0].el));
  f = atan(trunc(v))[2u];
  bool x_49 = (f > -(x_9.x_GLF_uniform_float_values[0].el));
  x_57 = x_49;
  if (x_49) {
    x_56 = (f < -(x_9.x_GLF_uniform_float_values[1].el));
    x_57 = x_56;
  }
  if (x_57) {
    float v_3 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_4 = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_5 = float(x_6.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_3, v_4, v_5, float(x_6.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[0].el));
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
