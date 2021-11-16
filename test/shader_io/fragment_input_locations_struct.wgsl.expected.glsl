SKIP: FAILED

#version 310 es
precision mediump float;

struct FragmentInputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
};
struct tint_symbol_2 {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
};

void tint_symbol_inner(FragmentInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  vec4 v = inputs.loc3;
}

void tint_symbol(tint_symbol_2 tint_symbol_1) {
  FragmentInputs tint_symbol_3 = FragmentInputs(tint_symbol_1.loc0, tint_symbol_1.loc1, tint_symbol_1.loc2, tint_symbol_1.loc3);
  tint_symbol_inner(tint_symbol_3);
  return;
}
in int loc0;
in uint loc1;
in float loc2;
in vec4 loc3;
void main() {
  tint_symbol_2 inputs;
  inputs.loc0 = loc0;
  inputs.loc1 = loc1;
  inputs.loc2 = loc2;
  inputs.loc3 = loc3;
  tint_symbol(inputs);
}


Error parsing GLSL shader:
ERROR: 0:29: 'int' : must be qualified as flat in
ERROR: 0:29: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



