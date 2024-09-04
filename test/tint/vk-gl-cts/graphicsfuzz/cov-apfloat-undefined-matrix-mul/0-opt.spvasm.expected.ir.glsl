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
  float v = float(x_6.x_GLF_uniform_int_values[4].el);
  float v_1 = float(x_6.x_GLF_uniform_int_values[5].el);
  vec4 v_2 = vec4(v, v_1, float(x_6.x_GLF_uniform_int_values[6].el), 4.0f);
  float v_3 = float(x_6.x_GLF_uniform_int_values[10].el);
  float v_4 = float(x_6.x_GLF_uniform_int_values[7].el);
  vec4 v_5 = vec4(v_3, v_4, float(x_6.x_GLF_uniform_int_values[8].el), 8.0f);
  float v_6 = float(x_6.x_GLF_uniform_int_values[11].el);
  float v_7 = float(x_6.x_GLF_uniform_int_values[1].el);
  float v_8 = float(x_6.x_GLF_uniform_int_values[2].el);
  m0 = mat3x4(v_2, v_5, vec4(v_6, v_7, v_8, float(x_6.x_GLF_uniform_int_values[3].el)));
  float x_104 = float(x_6.x_GLF_uniform_int_values[4].el);
  vec4 v_9 = vec4(x_104, 0.0f, 0.0f, 0.0f);
  vec4 v_10 = vec4(0.0f, x_104, 0.0f, 0.0f);
  m1 = mat3x4(v_9, v_10, vec4(0.0f, 0.0f, x_104, 0.0f));
  undefined = vec3(2.0f);
  vec3 v_11 = vec3(float(x_6.x_GLF_uniform_int_values[4].el));
  defined = ldexp(v_11, ivec3(x_6.x_GLF_uniform_int_values[0].el));
  v0 = (m0 * undefined);
  v1 = (m1 * undefined);
  v2 = (m0 * defined);
  v3 = (m1 * defined);
  if ((v2.x > v3.x)) {
    float v_12 = float(x_6.x_GLF_uniform_int_values[4].el);
    float v_13 = float(x_6.x_GLF_uniform_int_values[9].el);
    float v_14 = float(x_6.x_GLF_uniform_int_values[9].el);
    x_GLF_color = vec4(v_12, v_13, v_14, float(x_6.x_GLF_uniform_int_values[4].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[9].el));
  }
  if ((v0.x < v1.x)) {
    x_GLF_color[1u] = x_15.x_GLF_uniform_float_values[0].el;
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
