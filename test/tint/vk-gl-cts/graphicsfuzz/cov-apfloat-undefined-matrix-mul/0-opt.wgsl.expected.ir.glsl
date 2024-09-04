SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[12];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_15;
void main_1() {
  mat3x4 m0 = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  mat3x4 m1 = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec3 undefined = vec3(0.0f);
  vec3 defined = vec3(0.0f);
  vec4 v0 = vec4(0.0f);
  vec4 v1 = vec4(0.0f);
  vec4 v2 = vec4(0.0f);
  vec4 v3 = vec4(0.0f);
  int x_17 = x_6.x_GLF_uniform_int_values[4].el;
  int x_18 = x_6.x_GLF_uniform_int_values[5].el;
  int x_19 = x_6.x_GLF_uniform_int_values[6].el;
  int x_20 = x_6.x_GLF_uniform_int_values[10].el;
  int x_21 = x_6.x_GLF_uniform_int_values[7].el;
  int x_22 = x_6.x_GLF_uniform_int_values[8].el;
  int x_23 = x_6.x_GLF_uniform_int_values[11].el;
  int x_24 = x_6.x_GLF_uniform_int_values[1].el;
  int x_25 = x_6.x_GLF_uniform_int_values[2].el;
  int x_26 = x_6.x_GLF_uniform_int_values[3].el;
  float v = float(x_17);
  float v_1 = float(x_18);
  vec4 v_2 = vec4(v, v_1, float(x_19), 4.0f);
  float v_3 = float(x_20);
  float v_4 = float(x_21);
  vec4 v_5 = vec4(v_3, v_4, float(x_22), 8.0f);
  float v_6 = float(x_23);
  float v_7 = float(x_24);
  float v_8 = float(x_25);
  m0 = mat3x4(v_2, v_5, vec4(v_6, v_7, v_8, float(x_26)));
  int x_27 = x_6.x_GLF_uniform_int_values[4].el;
  float x_104 = float(x_27);
  vec4 v_9 = vec4(x_104, 0.0f, 0.0f, 0.0f);
  vec4 v_10 = vec4(0.0f, x_104, 0.0f, 0.0f);
  m1 = mat3x4(v_9, v_10, vec4(0.0f, 0.0f, x_104, 0.0f));
  undefined = vec3(2.0f);
  int x_28 = x_6.x_GLF_uniform_int_values[4].el;
  float x_111 = float(x_28);
  int x_29 = x_6.x_GLF_uniform_int_values[0].el;
  vec3 v_11 = vec3(x_111, x_111, x_111);
  defined = ldexp(v_11, ivec3(x_29, x_29, x_29));
  mat3x4 x_116 = m0;
  vec3 x_117 = undefined;
  v0 = (x_116 * x_117);
  mat3x4 x_119 = m1;
  vec3 x_120 = undefined;
  v1 = (x_119 * x_120);
  mat3x4 x_122 = m0;
  vec3 x_123 = defined;
  v2 = (x_122 * x_123);
  mat3x4 x_125 = m1;
  vec3 x_126 = defined;
  v3 = (x_125 * x_126);
  float x_129 = v2.x;
  float x_131 = v3.x;
  if ((x_129 > x_131)) {
    int x_30 = x_6.x_GLF_uniform_int_values[4].el;
    int x_31 = x_6.x_GLF_uniform_int_values[9].el;
    int x_32 = x_6.x_GLF_uniform_int_values[9].el;
    int x_33 = x_6.x_GLF_uniform_int_values[4].el;
    float v_12 = float(x_30);
    float v_13 = float(x_31);
    float v_14 = float(x_32);
    x_GLF_color = vec4(v_12, v_13, v_14, float(x_33));
  } else {
    int x_34 = x_6.x_GLF_uniform_int_values[9].el;
    float x_146 = float(x_34);
    x_GLF_color = vec4(x_146, x_146, x_146, x_146);
  }
  float x_149 = v0.x;
  float x_151 = v1.x;
  if ((x_149 < x_151)) {
    float x_156 = x_15.x_GLF_uniform_float_values[0].el;
    x_GLF_color[1u] = x_156;
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
