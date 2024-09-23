SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
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
ivec2 tint_div_v2i32(ivec2 lhs, ivec2 rhs) {
  int v_1 = ((((rhs == ivec2(0)) | ((lhs == ivec2((-2147483647 - 1))) & (rhs == ivec2(-1)))).x) ? (ivec2(1).x) : (rhs.x));
  return (lhs / ivec2(v_1, ((((rhs == ivec2(0)) | ((lhs == ivec2((-2147483647 - 1))) & (rhs == ivec2(-1)))).y) ? (ivec2(1).y) : (rhs.y))));
}
void main_1() {
  int a = 0;
  a = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  ivec2 v_2 = ivec2(a);
  int v_3 = tint_div_v2i32(v_2, ivec2(v.tint_symbol_1.x_GLF_uniform_int_values[2].el, 63677))[1u];
  if ((v_3 == v.tint_symbol_1.x_GLF_uniform_int_values[0].el)) {
    float v_4 = float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el);
    float v_5 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
    float v_6 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_4, v_5, v_6, float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el));
  } else {
    x_GLF_color = vec4(float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el));
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
