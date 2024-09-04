SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[7];
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
  bool x_75 = false;
  bool x_76 = false;
  bool x_92 = false;
  bool x_93 = false;
  bool x_109 = false;
  bool x_110 = false;
  a = packUnorm2x16(vec2(x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[1].el));
  v1 = unpackSnorm4x8(a);
  E = 0.00999999977648258209f;
  float v = abs((v1[x_10.x_GLF_uniform_int_values[2].el] - (x_6.x_GLF_uniform_float_values[2].el / x_6.x_GLF_uniform_float_values[3].el)));
  bool x_60 = (v < E);
  x_76 = x_60;
  if (x_60) {
    float v_1 = abs((v1[x_10.x_GLF_uniform_int_values[1].el] - (x_6.x_GLF_uniform_float_values[4].el / x_6.x_GLF_uniform_float_values[3].el)));
    x_75 = (v_1 < E);
    x_76 = x_75;
  }
  x_93 = x_76;
  if (x_76) {
    float v_2 = abs((v1[x_10.x_GLF_uniform_int_values[3].el] - (-(x_6.x_GLF_uniform_float_values[5].el) / x_6.x_GLF_uniform_float_values[3].el)));
    x_92 = (v_2 < E);
    x_93 = x_92;
  }
  x_110 = x_93;
  if (x_93) {
    float v_3 = abs((v1[x_10.x_GLF_uniform_int_values[0].el] - (-(x_6.x_GLF_uniform_float_values[6].el) / x_6.x_GLF_uniform_float_values[3].el)));
    x_109 = (v_3 < E);
    x_110 = x_109;
  }
  if (x_110) {
    float v_4 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_5 = float(x_10.x_GLF_uniform_int_values[2].el);
    float v_6 = float(x_10.x_GLF_uniform_int_values[2].el);
    x_GLF_color = vec4(v_4, v_5, v_6, float(x_10.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[5].el);
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
