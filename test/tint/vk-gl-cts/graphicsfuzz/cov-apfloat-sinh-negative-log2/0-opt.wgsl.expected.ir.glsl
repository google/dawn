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
  float undefined = 0.0f;
  bool x_45 = false;
  bool x_46_phi = false;
  undefined = 1.17520117759704589844f;
  int x_10 = x_6.x_GLF_uniform_int_values[0].el;
  bool x_38 = (1 == x_10);
  x_46_phi = x_38;
  if (!(x_38)) {
    float x_42 = undefined;
    float x_44 = x_8.x_GLF_uniform_float_values[0].el;
    x_45 = (x_42 > x_44);
    x_46_phi = x_45;
  }
  bool x_46 = x_46_phi;
  if (x_46) {
    int x_12 = x_6.x_GLF_uniform_int_values[0].el;
    int x_13 = x_6.x_GLF_uniform_int_values[1].el;
    int x_14 = x_6.x_GLF_uniform_int_values[1].el;
    int x_15 = x_6.x_GLF_uniform_int_values[0].el;
    float v = float(x_12);
    float v_1 = float(x_13);
    float v_2 = float(x_14);
    x_GLF_color = vec4(v, v_1, v_2, float(x_15));
  } else {
    int x_16 = x_6.x_GLF_uniform_int_values[1].el;
    float x_60 = float(x_16);
    x_GLF_color = vec4(x_60, x_60, x_60, x_60);
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
