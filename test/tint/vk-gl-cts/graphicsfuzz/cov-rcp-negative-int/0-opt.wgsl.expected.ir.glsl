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
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
void main_1() {
  int a = 0;
  a = -7563;
  int x_25 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  int x_26 = a;
  int x_29 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
  if ((tint_div_i32(x_25, x_26) == x_29)) {
    int x_35 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    int x_38 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    int x_41 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    int x_44 = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
    float v_1 = float(x_35);
    float v_2 = float(x_38);
    float v_3 = float(x_41);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_44));
  } else {
    int x_48 = v.tint_symbol_1.x_GLF_uniform_int_values[0].el;
    float x_49 = float(x_48);
    x_GLF_color = vec4(x_49, x_49, x_49, x_49);
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
ERROR: 0:25: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:25: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:25: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
