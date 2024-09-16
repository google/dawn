SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


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

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v_1;
layout(binding = 1, std140)
uniform tint_symbol_4_1_ubo {
  buf1 tint_symbol_3;
} v_2;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  mat3 m = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  int a = 0;
  vec3 arr[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  vec3 v = vec3(0.0f);
  int x_45 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  float x_46 = float(x_45);
  vec3 v_3 = vec3(x_46, 0.0f, 0.0f);
  vec3 v_4 = vec3(0.0f, x_46, 0.0f);
  m = mat3(v_3, v_4, vec3(0.0f, 0.0f, x_46));
  int x_52 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  a = x_52;
  int x_53 = a;
  int x_54 = a;
  float x_56 = v_2.tint_symbol_3.x_GLF_uniform_float_values[0].el;
  m[x_53][x_54] = x_56;
  vec3 x_59 = m[1];
  vec3 x_61 = m[1];
  arr = vec3[2](x_59, x_61);
  float x_64 = v_2.tint_symbol_3.x_GLF_uniform_float_values[1].el;
  v = vec3(x_64, x_64, x_64);
  int x_66 = a;
  vec3 x_68 = arr[x_66];
  vec3 x_69 = v;
  v = (x_69 + x_68);
  vec3 x_71 = v;
  int x_73 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_76 = v_1.tint_symbol_1.x_GLF_uniform_int_values[2].el;
  int x_79 = v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  float v_5 = float(x_73);
  float v_6 = float(x_76);
  if (all((x_71 == vec3(v_5, v_6, float(x_79))))) {
    int x_88 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    int x_91 = v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el;
    int x_94 = v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el;
    int x_97 = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    float v_7 = float(x_88);
    float v_8 = float(x_91);
    float v_9 = float(x_94);
    x_GLF_color = vec4(v_7, v_8, v_9, float(x_97));
  } else {
    int x_101 = v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el;
    float x_102 = float(x_101);
    x_GLF_color = vec4(x_102, x_102, x_102, x_102);
  }
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:67: 'all' : no matching overloaded function found 
ERROR: 0:67: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
