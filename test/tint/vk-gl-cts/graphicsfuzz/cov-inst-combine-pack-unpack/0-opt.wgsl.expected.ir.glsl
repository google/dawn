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
  bool x_92 = false;
  bool x_109 = false;
  bool x_76_phi = false;
  bool x_93_phi = false;
  bool x_110_phi = false;
  float x_41 = x_6.x_GLF_uniform_float_values[0].el;
  float x_43 = x_6.x_GLF_uniform_float_values[1].el;
  a = packUnorm2x16(vec2(x_41, x_43));
  uint x_46 = a;
  v1 = unpackSnorm4x8(x_46);
  E = 0.00999999977648258209f;
  int x_49 = x_10.x_GLF_uniform_int_values[2].el;
  float x_51 = v1[x_49];
  float x_53 = x_6.x_GLF_uniform_float_values[2].el;
  float x_55 = x_6.x_GLF_uniform_float_values[3].el;
  float x_59 = E;
  bool x_60 = (abs((x_51 - (x_53 / x_55))) < x_59);
  x_76_phi = x_60;
  if (x_60) {
    int x_64 = x_10.x_GLF_uniform_int_values[1].el;
    float x_66 = v1[x_64];
    float x_68 = x_6.x_GLF_uniform_float_values[4].el;
    float x_70 = x_6.x_GLF_uniform_float_values[3].el;
    float x_74 = E;
    x_75 = (abs((x_66 - (x_68 / x_70))) < x_74);
    x_76_phi = x_75;
  }
  bool x_76 = x_76_phi;
  x_93_phi = x_76;
  if (x_76) {
    int x_80 = x_10.x_GLF_uniform_int_values[3].el;
    float x_82 = v1[x_80];
    float x_84 = x_6.x_GLF_uniform_float_values[5].el;
    float x_87 = x_6.x_GLF_uniform_float_values[3].el;
    float x_91 = E;
    x_92 = (abs((x_82 - (-(x_84) / x_87))) < x_91);
    x_93_phi = x_92;
  }
  bool x_93 = x_93_phi;
  x_110_phi = x_93;
  if (x_93) {
    int x_97 = x_10.x_GLF_uniform_int_values[0].el;
    float x_99 = v1[x_97];
    float x_101 = x_6.x_GLF_uniform_float_values[6].el;
    float x_104 = x_6.x_GLF_uniform_float_values[3].el;
    float x_108 = E;
    x_109 = (abs((x_99 - (-(x_101) / x_104))) < x_108);
    x_110_phi = x_109;
  }
  bool x_110 = x_110_phi;
  if (x_110) {
    int x_115 = x_10.x_GLF_uniform_int_values[1].el;
    int x_118 = x_10.x_GLF_uniform_int_values[2].el;
    int x_121 = x_10.x_GLF_uniform_int_values[2].el;
    int x_124 = x_10.x_GLF_uniform_int_values[1].el;
    float v = float(x_115);
    float v_1 = float(x_118);
    float v_2 = float(x_121);
    x_GLF_color = vec4(v, v_1, v_2, float(x_124));
  } else {
    float x_128 = x_6.x_GLF_uniform_float_values[5].el;
    x_GLF_color = vec4(x_128, x_128, x_128, x_128);
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
