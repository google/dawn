SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct main_out {
  vec4 color_out_1;
};

vec4 frag_color = vec4(0.0f);
vec4 color_out = vec4(0.0f);
layout(location = 1) in vec4 tint_symbol_loc1_Input;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
ivec2 tint_v2f32_to_v2i32(vec2 value) {
  ivec2 v = ivec2(value);
  int v_1 = (((value >= vec2(-2147483648.0f)).x) ? (v.x) : (ivec2((-2147483647 - 1)).x));
  ivec2 v_2 = ivec2(v_1, (((value >= vec2(-2147483648.0f)).y) ? (v.y) : (ivec2((-2147483647 - 1)).y)));
  int v_3 = (((value <= vec2(2147483520.0f)).x) ? (v_2.x) : (ivec2(2147483647).x));
  return ivec2(v_3, (((value <= vec2(2147483520.0f)).y) ? (v_2.y) : (ivec2(2147483647).y)));
}
void main_1() {
  ivec2 iv = ivec2(0);
  vec4 x_28 = frag_color;
  iv = tint_v2f32_to_v2i32((vec2(x_28[0u], x_28[1u]) * 256.0f));
  int x_33 = iv.y;
  if (((tint_div_i32(x_33, 2) & 64) == 64)) {
    color_out = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    color_out = vec4(0.0f, 1.0f, 1.0f, 1.0f);
  }
}
main_out tint_symbol_inner(vec4 frag_color_param) {
  frag_color = frag_color_param;
  main_1();
  return main_out(color_out);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner(tint_symbol_loc1_Input).color_out_1;
}
error: Error parsing GLSL shader:
ERROR: 0:15: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:15: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
