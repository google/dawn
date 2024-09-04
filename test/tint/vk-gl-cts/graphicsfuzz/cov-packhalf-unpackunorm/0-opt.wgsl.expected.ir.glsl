SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[4];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_8;
uniform buf1 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  uint a = 0u;
  vec4 values = vec4(0.0f);
  vec4 r = vec4(0.0f);
  bool x_85 = false;
  bool x_101 = false;
  bool x_117 = false;
  bool x_86_phi = false;
  bool x_102_phi = false;
  bool x_118_phi = false;
  a = 1006648320u;
  uint x_38 = a;
  values = unpackUnorm4x8(x_38);
  float x_41 = x_8.x_GLF_uniform_float_values[3].el;
  float x_43 = x_8.x_GLF_uniform_float_values[1].el;
  float x_45 = x_8.x_GLF_uniform_float_values[0].el;
  float x_48 = x_8.x_GLF_uniform_float_values[3].el;
  float x_50 = x_8.x_GLF_uniform_float_values[0].el;
  float x_53 = x_8.x_GLF_uniform_float_values[1].el;
  float x_55 = x_8.x_GLF_uniform_float_values[0].el;
  r = vec4(x_41, (x_43 / x_45), (x_48 / x_50), (x_53 / x_55));
  int x_59 = x_10.x_GLF_uniform_int_values[0].el;
  float x_61 = values[x_59];
  int x_63 = x_10.x_GLF_uniform_int_values[0].el;
  float x_65 = r[x_63];
  float x_69 = x_8.x_GLF_uniform_float_values[2].el;
  bool x_70 = (abs((x_61 - x_65)) < x_69);
  x_86_phi = x_70;
  if (x_70) {
    int x_74 = x_10.x_GLF_uniform_int_values[1].el;
    float x_76 = values[x_74];
    int x_78 = x_10.x_GLF_uniform_int_values[1].el;
    float x_80 = r[x_78];
    float x_84 = x_8.x_GLF_uniform_float_values[2].el;
    x_85 = (abs((x_76 - x_80)) < x_84);
    x_86_phi = x_85;
  }
  bool x_86 = x_86_phi;
  x_102_phi = x_86;
  if (x_86) {
    int x_90 = x_10.x_GLF_uniform_int_values[3].el;
    float x_92 = values[x_90];
    int x_94 = x_10.x_GLF_uniform_int_values[3].el;
    float x_96 = r[x_94];
    float x_100 = x_8.x_GLF_uniform_float_values[2].el;
    x_101 = (abs((x_92 - x_96)) < x_100);
    x_102_phi = x_101;
  }
  bool x_102 = x_102_phi;
  x_118_phi = x_102;
  if (x_102) {
    int x_106 = x_10.x_GLF_uniform_int_values[2].el;
    float x_108 = values[x_106];
    int x_110 = x_10.x_GLF_uniform_int_values[2].el;
    float x_112 = r[x_110];
    float x_116 = x_8.x_GLF_uniform_float_values[2].el;
    x_117 = (abs((x_108 - x_112)) < x_116);
    x_118_phi = x_117;
  }
  bool x_118 = x_118_phi;
  if (x_118) {
    int x_123 = x_10.x_GLF_uniform_int_values[1].el;
    int x_126 = x_10.x_GLF_uniform_int_values[0].el;
    int x_129 = x_10.x_GLF_uniform_int_values[0].el;
    int x_132 = x_10.x_GLF_uniform_int_values[1].el;
    float v = float(x_123);
    float v_1 = float(x_126);
    float v_2 = float(x_129);
    x_GLF_color = vec4(v, v_1, v_2, float(x_132));
  } else {
    int x_136 = x_10.x_GLF_uniform_int_values[0].el;
    float x_137 = float(x_136);
    x_GLF_color = vec4(x_137, x_137, x_137, x_137);
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
