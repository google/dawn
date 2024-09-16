SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[4];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};

vec4 x_GLF_color = vec4(0.0f);
layout(binding = 1, std140)
uniform tint_symbol_2_1_ubo {
  buf1 tint_symbol_1;
} v;
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v_1;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
float func_f1_(inout float b) {
  x_GLF_color = vec4(v.tint_symbol_1.x_GLF_uniform_float_values[0].el, v.tint_symbol_1.x_GLF_uniform_float_values[0].el, v.tint_symbol_1.x_GLF_uniform_float_values[1].el, 1.0f);
  x_GLF_color = x_GLF_color;
  if ((v.tint_symbol_1.x_GLF_uniform_float_values[0].el >= b)) {
    float x_104 = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
    return x_104;
  }
  float x_106 = v.tint_symbol_1.x_GLF_uniform_float_values[2].el;
  return x_106;
}
void main_1() {
  float a = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  bool x_71 = false;
  bool x_72 = false;
  param = v.tint_symbol_1.x_GLF_uniform_float_values[0].el;
  float x_45 = func_f1_(param);
  a = x_45;
  param_1 = (v.tint_symbol_1.x_GLF_uniform_float_values[0].el + v.tint_symbol_1.x_GLF_uniform_float_values[0].el);
  float x_51 = func_f1_(param_1);
  a = (a + x_51);
  bool x_57 = (a == v.tint_symbol_1.x_GLF_uniform_float_values[3].el);
  x_72 = x_57;
  if (x_57) {
    vec4 v_2 = x_GLF_color;
    x_71 = all((v_2 == vec4(v.tint_symbol_1.x_GLF_uniform_float_values[0].el, v.tint_symbol_1.x_GLF_uniform_float_values[0].el, v.tint_symbol_1.x_GLF_uniform_float_values[1].el, v.tint_symbol_1.x_GLF_uniform_float_values[0].el)));
    x_72 = x_71;
  }
  if (x_72) {
    float v_3 = float(v_1.tint_symbol_3.x_GLF_uniform_int_values[0].el);
    float v_4 = float(v_1.tint_symbol_3.x_GLF_uniform_int_values[1].el);
    float v_5 = float(v_1.tint_symbol_3.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_3, v_4, v_5, float(v_1.tint_symbol_3.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(v_1.tint_symbol_3.x_GLF_uniform_int_values[1].el));
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
ERROR: 0:62: 'all' : no matching overloaded function found 
ERROR: 0:62: 'assign' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:62: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
