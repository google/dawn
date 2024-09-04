SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[4];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  uint a = 0u;
  vec4 v1 = vec4(0.0f);
  float E = 0.0f;
  bool x_69 = false;
  bool x_70 = false;
  bool x_85 = false;
  bool x_86 = false;
  bool x_101 = false;
  bool x_102 = false;
  a = packSnorm4x8(vec4(x_6.x_GLF_uniform_float_values[1].el));
  v1 = unpackUnorm4x8(a);
  E = x_6.x_GLF_uniform_float_values[0].el;
  float v = abs((v1[x_10.x_GLF_uniform_int_values[1].el] - (x_6.x_GLF_uniform_float_values[2].el / x_6.x_GLF_uniform_float_values[3].el)));
  bool x_54 = (v < E);
  x_70 = x_54;
  if (x_54) {
    float v_1 = abs((v1[x_10.x_GLF_uniform_int_values[0].el] - (x_6.x_GLF_uniform_float_values[2].el / x_6.x_GLF_uniform_float_values[3].el)));
    x_69 = (v_1 < E);
    x_70 = x_69;
  }
  x_86 = x_70;
  if (x_70) {
    float v_2 = abs((v1[x_10.x_GLF_uniform_int_values[3].el] - (x_6.x_GLF_uniform_float_values[2].el / x_6.x_GLF_uniform_float_values[3].el)));
    x_85 = (v_2 < E);
    x_86 = x_85;
  }
  x_102 = x_86;
  if (x_86) {
    float v_3 = abs((v1[x_10.x_GLF_uniform_int_values[2].el] - (x_6.x_GLF_uniform_float_values[2].el / x_6.x_GLF_uniform_float_values[3].el)));
    x_101 = (v_3 < E);
    x_102 = x_101;
  }
  if (x_102) {
    float v_4 = float(x_10.x_GLF_uniform_int_values[0].el);
    float v_5 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_6 = float(x_10.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_4, v_5, v_6, float(x_10.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_10.x_GLF_uniform_int_values[1].el));
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
