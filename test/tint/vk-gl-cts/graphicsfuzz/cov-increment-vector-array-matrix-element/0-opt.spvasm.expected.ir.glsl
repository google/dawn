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
  float x_46 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el);
  vec3 v_3 = vec3(x_46, 0.0f, 0.0f);
  vec3 v_4 = vec3(0.0f, x_46, 0.0f);
  m = mat3(v_3, v_4, vec3(0.0f, 0.0f, x_46));
  a = v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  int x_53 = a;
  int x_54 = a;
  m[x_53][x_54] = v_2.tint_symbol_3.x_GLF_uniform_float_values[0].el;
  arr = vec3[2](m[1], m[1]);
  v = vec3(v_2.tint_symbol_3.x_GLF_uniform_float_values[1].el);
  v = (v + arr[a]);
  vec3 v_5 = v;
  float v_6 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el);
  float v_7 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[2].el);
  if (all((v_5 == vec3(v_6, v_7, float(v_1.tint_symbol_1.x_GLF_uniform_int_values[1].el))))) {
    float v_8 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el);
    float v_9 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el);
    float v_10 = float(v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el);
    x_GLF_color = vec4(v_8, v_9, v_10, float(v_1.tint_symbol_1.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(v_1.tint_symbol_1.x_GLF_uniform_int_values[3].el));
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
ERROR: 0:55: 'all' : no matching overloaded function found 
ERROR: 0:55: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
