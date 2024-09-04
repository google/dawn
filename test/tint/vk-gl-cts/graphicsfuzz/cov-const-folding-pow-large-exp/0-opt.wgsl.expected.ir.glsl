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
  bool x_49_phi = false;
  f = 1626.509033203125f;
  int x_35 = x_6.x_GLF_uniform_int_values[0].el;
  int x_37 = x_6.x_GLF_uniform_int_values[0].el;
  int x_39 = x_6.x_GLF_uniform_int_values[1].el;
  bool x_41 = (x_35 == (x_37 + x_39));
  x_49_phi = x_41;
  if (!(x_41)) {
    float x_45 = f;
    float x_47 = x_8.x_GLF_uniform_float_values[0].el;
    x_48 = (x_45 > x_47);
    x_49_phi = x_48;
  }
  bool x_49 = x_49_phi;
  if (x_49) {
    int x_54 = x_6.x_GLF_uniform_int_values[0].el;
    int x_57 = x_6.x_GLF_uniform_int_values[1].el;
    int x_60 = x_6.x_GLF_uniform_int_values[1].el;
    int x_63 = x_6.x_GLF_uniform_int_values[0].el;
    float v = float(x_54);
    float v_1 = float(x_57);
    float v_2 = float(x_60);
    x_GLF_color = vec4(v, v_1, v_2, float(x_63));
  } else {
    int x_67 = x_6.x_GLF_uniform_int_values[1].el;
    float x_68 = float(x_67);
    x_GLF_color = vec4(x_68, x_68, x_68, x_68);
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
