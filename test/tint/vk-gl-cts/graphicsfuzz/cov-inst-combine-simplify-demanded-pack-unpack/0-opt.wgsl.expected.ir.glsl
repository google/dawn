SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_8;
uniform buf0 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  uint a = 0u;
  vec4 v1 = vec4(0.0f);
  float E = 0.0f;
  bool x_69 = false;
  bool x_85 = false;
  bool x_101 = false;
  bool x_70_phi = false;
  bool x_86_phi = false;
  bool x_102_phi = false;
  a = 1006648320u;
  uint x_40 = a;
  v1 = unpackSnorm4x8(x_40);
  E = 0.00999999977648258209f;
  int x_43 = x_8.x_GLF_uniform_int_values[1].el;
  float x_45 = v1[x_43];
  float x_47 = x_10.x_GLF_uniform_float_values[0].el;
  float x_49 = x_10.x_GLF_uniform_float_values[1].el;
  float x_53 = E;
  bool x_54 = (abs((x_45 - (x_47 / x_49))) < x_53);
  x_70_phi = x_54;
  if (x_54) {
    int x_58 = x_8.x_GLF_uniform_int_values[0].el;
    float x_60 = v1[x_58];
    float x_62 = x_10.x_GLF_uniform_float_values[2].el;
    float x_64 = x_10.x_GLF_uniform_float_values[1].el;
    float x_68 = E;
    x_69 = (abs((x_60 - (x_62 / x_64))) < x_68);
    x_70_phi = x_69;
  }
  bool x_70 = x_70_phi;
  x_86_phi = x_70;
  if (x_70) {
    int x_74 = x_8.x_GLF_uniform_int_values[2].el;
    float x_76 = v1[x_74];
    float x_78 = x_10.x_GLF_uniform_float_values[0].el;
    float x_80 = x_10.x_GLF_uniform_float_values[1].el;
    float x_84 = E;
    x_85 = (abs((x_76 - (x_78 / x_80))) < x_84);
    x_86_phi = x_85;
  }
  bool x_86 = x_86_phi;
  x_102_phi = x_86;
  if (x_86) {
    int x_90 = x_8.x_GLF_uniform_int_values[3].el;
    float x_92 = v1[x_90];
    float x_94 = x_10.x_GLF_uniform_float_values[2].el;
    float x_96 = x_10.x_GLF_uniform_float_values[1].el;
    float x_100 = E;
    x_101 = (abs((x_92 - (x_94 / x_96))) < x_100);
    x_102_phi = x_101;
  }
  bool x_102 = x_102_phi;
  if (x_102) {
    int x_107 = x_8.x_GLF_uniform_int_values[0].el;
    int x_110 = x_8.x_GLF_uniform_int_values[1].el;
    int x_113 = x_8.x_GLF_uniform_int_values[1].el;
    int x_116 = x_8.x_GLF_uniform_int_values[0].el;
    float v = float(x_107);
    float v_1 = float(x_110);
    float v_2 = float(x_113);
    x_GLF_color = vec4(v, v_1, v_2, float(x_116));
  } else {
    int x_120 = x_8.x_GLF_uniform_int_values[1].el;
    float x_122 = v1[x_120];
    x_GLF_color = vec4(x_122, x_122, x_122, x_122);
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
