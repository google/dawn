SKIP: FAILED

#version 310 es
precision mediump float;

vec4 x_2 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
int x_3 = 0;
int x_4 = 0;
uniform highp writeonly iimage2D x_5;

void main_1() {
  x_4 = 1;
  vec4 x_23 = x_2;
  int x_27 = int(x_23.x);
  int x_28 = int(x_23.y);
  if (((((x_27 & 1) + (x_28 & 1)) + x_3) == int(x_23.z))) {
  }
  imageStore(x_5, ivec2(x_27, x_28), ivec4(x_27, 0, 0, 0));
  return;
}

struct main_out {
  int x_4_1;
};
struct tint_symbol_2 {
  int x_3_param;
  vec4 x_2_param;
};
struct tint_symbol_3 {
  int x_4_1;
};

main_out tint_symbol_inner(vec4 x_2_param, int x_3_param) {
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  main_out tint_symbol_4 = main_out(x_4);
  return tint_symbol_4;
}

tint_symbol_3 tint_symbol(tint_symbol_2 tint_symbol_1) {
  main_out inner_result = tint_symbol_inner(tint_symbol_1.x_2_param, tint_symbol_1.x_3_param);
  tint_symbol_3 wrapper_result = tint_symbol_3(0);
  wrapper_result.x_4_1 = inner_result.x_4_1;
  return wrapper_result;
}
in int x_3_param;
out int x_4_1;
void main() {
  tint_symbol_2 inputs;
  inputs.x_3_param = x_3_param;
  inputs.x_2_param = gl_FragCoord;
  tint_symbol_3 outputs;
  outputs = tint_symbol(inputs);
  x_4_1 = outputs.x_4_1;
}


Error parsing GLSL shader:
ERROR: 0:45: 'int' : must be qualified as flat in
ERROR: 0:45: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



