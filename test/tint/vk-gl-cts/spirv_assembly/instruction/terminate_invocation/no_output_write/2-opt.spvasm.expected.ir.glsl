SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct main_out {
  int out_data_1;
};

vec4 tint_symbol = vec4(0.0f);
int out_data = 0;
layout(location = 0) out int tint_symbol_1_loc0_Output;
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
void main_1() {
  bool x_is_odd = false;
  bool y_is_odd = false;
  x_is_odd = ((tint_f32_to_i32(tint_symbol.x) & 1) == 1);
  y_is_odd = ((tint_f32_to_i32(tint_symbol.y) & 1) == 1);
  out_data = (((x_is_odd | y_is_odd)) ? (1) : (0));
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(out_data);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).out_data_1;
}
error: Error parsing GLSL shader:
ERROR: 0:21: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
