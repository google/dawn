SKIP: FAILED

warning: integral user-defined fragment inputs must have a flat interpolation attribute
warning: integral user-defined fragment inputs must have a flat interpolation attribute
#version 310 es
precision mediump float;

uint x_1 = 0u;
uint x_2 = 0u;
uint x_3 = 0u;
uint x_4 = 0u;

void main_1() {
  return;
}

struct main_out {
  uint x_2_1;
  uint x_4_1;
};
struct tint_symbol_2 {
  uint x_1_param;
  uint x_3_param;
};
struct tint_symbol_3 {
  uint x_2_1;
  uint x_4_1;
};

main_out tint_symbol_inner(uint x_1_param, uint x_3_param) {
  x_1 = x_1_param;
  x_3 = x_3_param;
  main_1();
  main_out tint_symbol_4 = main_out(x_2, x_4);
  return tint_symbol_4;
}

tint_symbol_3 tint_symbol(tint_symbol_2 tint_symbol_1) {
  main_out inner_result = tint_symbol_inner(tint_symbol_1.x_1_param, tint_symbol_1.x_3_param);
  tint_symbol_3 wrapper_result = tint_symbol_3(0u, 0u);
  wrapper_result.x_2_1 = inner_result.x_2_1;
  wrapper_result.x_4_1 = inner_result.x_4_1;
  return wrapper_result;
}
in uint x_1_param;
in uint x_3_param;
out uint x_2_1;
out uint x_4_1;
void main() {
  tint_symbol_2 inputs;
  inputs.x_1_param = x_1_param;
  inputs.x_3_param = x_3_param;
  tint_symbol_3 outputs;
  outputs = tint_symbol(inputs);
  x_2_1 = outputs.x_2_1;
  x_4_1 = outputs.x_4_1;
}


Error parsing GLSL shader:
ERROR: 0:41: 'uint' : must be qualified as flat in
ERROR: 0:41: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



