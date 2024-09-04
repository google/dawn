SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct buf2 {
  int zero;
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf2 x_8;
uniform buf0 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float f = 0.0f;
  bool x_48 = false;
  bool x_49_phi = false;
  float x_33 = x_6.x_GLF_uniform_float_values[1].el;
  f = pow(-(x_33), 1.17520117759704589844f);
  float x_37 = f;
  float x_39 = x_6.x_GLF_uniform_float_values[0].el;
  bool x_40 = (x_37 == x_39);
  x_49_phi = x_40;
  if (!(x_40)) {
    int x_45 = x_8.zero;
    int x_47 = x_10.x_GLF_uniform_int_values[0].el;
    x_48 = (x_45 == x_47);
    x_49_phi = x_48;
  }
  bool x_49 = x_49_phi;
  if (x_49) {
    int x_54 = x_10.x_GLF_uniform_int_values[1].el;
    int x_57 = x_10.x_GLF_uniform_int_values[0].el;
    int x_60 = x_10.x_GLF_uniform_int_values[0].el;
    int x_63 = x_10.x_GLF_uniform_int_values[1].el;
    float v = float(x_54);
    float v_1 = float(x_57);
    float v_2 = float(x_60);
    x_GLF_color = vec4(v, v_1, v_2, float(x_63));
  } else {
    int x_67 = x_10.x_GLF_uniform_int_values[0].el;
    float x_68 = float(x_67);
    x_GLF_color = vec4(x_68, x_68, x_68, x_68);
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
