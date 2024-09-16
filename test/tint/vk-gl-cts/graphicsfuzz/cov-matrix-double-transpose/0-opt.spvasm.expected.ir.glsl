SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  mat2 m = mat2(vec2(0.0f), vec2(0.0f));
  float x_30 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
  vec2 v_1 = vec2(x_30, 0.0f);
  m = transpose(transpose(mat2(v_1, vec2(0.0f, x_30))));
  float x_39 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
  vec2 v_2 = vec2(x_39, 0.0f);
  mat2 x_42 = mat2(v_2, vec2(0.0f, x_39));
  bool v_3 = all((m[0u] == x_42[0u]));
  if ((v_3 & all((m[1u] == x_42[1u])))) {
    float v_4 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
    float v_5 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    float v_6 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_4, v_5, v_6, float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el));
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
ERROR: 0:32: 'all' : no matching overloaded function found 
ERROR: 0:32: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:32: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
