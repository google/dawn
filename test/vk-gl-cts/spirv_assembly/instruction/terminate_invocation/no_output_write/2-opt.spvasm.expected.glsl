SKIP: FAILED

#version 310 es
precision mediump float;

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
int out_data = 0;

void main_1() {
  bool x_is_odd = false;
  bool y_is_odd = false;
  float x_24 = tint_symbol.x;
  x_is_odd = ((int(x_24) & 1) == 1);
  float x_29 = tint_symbol.y;
  y_is_odd = ((int(x_29) & 1) == 1);
  out_data = ((x_is_odd | y_is_odd) ? 1 : 0);
  return;
}

struct main_out {
  int out_data_1;
};
struct tint_symbol_4 {
  vec4 tint_symbol_2;
};
struct tint_symbol_5 {
  int out_data_1;
};

main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out tint_symbol_6 = main_out(out_data);
  return tint_symbol_6;
}

tint_symbol_5 tint_symbol_1(tint_symbol_4 tint_symbol_3) {
  main_out inner_result = tint_symbol_1_inner(tint_symbol_3.tint_symbol_2);
  tint_symbol_5 wrapper_result = tint_symbol_5(0);
  wrapper_result.out_data_1 = inner_result.out_data_1;
  return wrapper_result;
}
out int out_data_1;
void main() {
  tint_symbol_4 inputs;
  inputs.tint_symbol_2 = gl_FragCoord;
  tint_symbol_5 outputs;
  outputs = tint_symbol_1(inputs);
  out_data_1 = outputs.out_data_1;
}


Error parsing GLSL shader:
ERROR: 0:14: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



