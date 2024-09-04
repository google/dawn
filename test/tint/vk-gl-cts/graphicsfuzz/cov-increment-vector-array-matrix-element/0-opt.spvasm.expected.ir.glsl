SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct strided_arr_1 {
  float el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  mat3 m = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  int a = 0;
  vec3 arr[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  vec3 v = vec3(0.0f);
  float x_46 = float(x_6.x_GLF_uniform_int_values[0].el);
  vec3 v_1 = vec3(x_46, 0.0f, 0.0f);
  vec3 v_2 = vec3(0.0f, x_46, 0.0f);
  m = mat3(v_1, v_2, vec3(0.0f, 0.0f, x_46));
  a = x_6.x_GLF_uniform_int_values[0].el;
  int x_53 = a;
  int x_54 = a;
  m[x_53][x_54] = x_9.x_GLF_uniform_float_values[0].el;
  arr = vec3[2](m[1], m[1]);
  v = vec3(x_9.x_GLF_uniform_float_values[1].el);
  v = (v + arr[a]);
  vec3 v_3 = v;
  float v_4 = float(x_6.x_GLF_uniform_int_values[1].el);
  float v_5 = float(x_6.x_GLF_uniform_int_values[2].el);
  if (all((v_3 == vec3(v_4, v_5, float(x_6.x_GLF_uniform_int_values[1].el))))) {
    float v_6 = float(x_6.x_GLF_uniform_int_values[0].el);
    float v_7 = float(x_6.x_GLF_uniform_int_values[3].el);
    float v_8 = float(x_6.x_GLF_uniform_int_values[3].el);
    x_GLF_color = vec4(v_6, v_7, v_8, float(x_6.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[3].el));
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
