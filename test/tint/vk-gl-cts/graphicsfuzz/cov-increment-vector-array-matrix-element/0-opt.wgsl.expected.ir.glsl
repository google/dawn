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
  int x_45 = x_6.x_GLF_uniform_int_values[0].el;
  float x_46 = float(x_45);
  vec3 v_1 = vec3(x_46, 0.0f, 0.0f);
  vec3 v_2 = vec3(0.0f, x_46, 0.0f);
  m = mat3(v_1, v_2, vec3(0.0f, 0.0f, x_46));
  int x_52 = x_6.x_GLF_uniform_int_values[0].el;
  a = x_52;
  int x_53 = a;
  int x_54 = a;
  float x_56 = x_9.x_GLF_uniform_float_values[0].el;
  m[x_53][x_54] = x_56;
  vec3 x_59 = m[1];
  vec3 x_61 = m[1];
  arr = vec3[2](x_59, x_61);
  float x_64 = x_9.x_GLF_uniform_float_values[1].el;
  v = vec3(x_64, x_64, x_64);
  int x_66 = a;
  vec3 x_68 = arr[x_66];
  vec3 x_69 = v;
  v = (x_69 + x_68);
  vec3 x_71 = v;
  int x_73 = x_6.x_GLF_uniform_int_values[1].el;
  int x_76 = x_6.x_GLF_uniform_int_values[2].el;
  int x_79 = x_6.x_GLF_uniform_int_values[1].el;
  float v_3 = float(x_73);
  float v_4 = float(x_76);
  if (all((x_71 == vec3(v_3, v_4, float(x_79))))) {
    int x_88 = x_6.x_GLF_uniform_int_values[0].el;
    int x_91 = x_6.x_GLF_uniform_int_values[3].el;
    int x_94 = x_6.x_GLF_uniform_int_values[3].el;
    int x_97 = x_6.x_GLF_uniform_int_values[0].el;
    float v_5 = float(x_88);
    float v_6 = float(x_91);
    float v_7 = float(x_94);
    x_GLF_color = vec4(v_5, v_6, v_7, float(x_97));
  } else {
    int x_101 = x_6.x_GLF_uniform_int_values[3].el;
    float x_102 = float(x_101);
    x_GLF_color = vec4(x_102, x_102, x_102, x_102);
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
