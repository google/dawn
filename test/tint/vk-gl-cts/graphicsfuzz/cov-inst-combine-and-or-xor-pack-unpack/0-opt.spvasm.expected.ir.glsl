SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
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


uniform buf0 x_6;
uniform buf1 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  uint a = 0u;
  vec4 v1 = vec4(0.0f);
  vec4 r = vec4(0.0f);
  bool x_85 = false;
  bool x_86 = false;
  bool x_97 = false;
  bool x_98 = false;
  bool x_109 = false;
  bool x_110 = false;
  a = packUnorm4x8(vec4(x_6.x_GLF_uniform_float_values[0].el));
  v1 = unpackSnorm4x8(a);
  r = vec4((-(x_6.x_GLF_uniform_float_values[0].el) / x_6.x_GLF_uniform_float_values[1].el), (-(x_6.x_GLF_uniform_float_values[0].el) / x_6.x_GLF_uniform_float_values[1].el), (-(x_6.x_GLF_uniform_float_values[0].el) / x_6.x_GLF_uniform_float_values[1].el), (-(x_6.x_GLF_uniform_float_values[0].el) / x_6.x_GLF_uniform_float_values[1].el));
  bool x_74 = (v1[x_10.x_GLF_uniform_int_values[1].el] == r[x_10.x_GLF_uniform_int_values[0].el]);
  x_86 = x_74;
  if (x_74) {
    x_85 = (v1[x_10.x_GLF_uniform_int_values[3].el] == r[x_10.x_GLF_uniform_int_values[2].el]);
    x_86 = x_85;
  }
  x_98 = x_86;
  if (x_86) {
    x_97 = (v1[x_10.x_GLF_uniform_int_values[2].el] == r[x_10.x_GLF_uniform_int_values[3].el]);
    x_98 = x_97;
  }
  x_110 = x_98;
  if (x_98) {
    x_109 = (v1[x_10.x_GLF_uniform_int_values[0].el] == r[x_10.x_GLF_uniform_int_values[1].el]);
    x_110 = x_109;
  }
  if (x_110) {
    float v = float(x_10.x_GLF_uniform_int_values[3].el);
    float v_1 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_10.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_10.x_GLF_uniform_int_values[3].el));
  } else {
    x_GLF_color = vec4(v1[x_10.x_GLF_uniform_int_values[1].el]);
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
