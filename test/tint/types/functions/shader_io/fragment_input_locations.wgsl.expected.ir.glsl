#version 310 es
precision highp float;
precision highp int;

layout(location = 0) flat in int tint_symbol_loc0_Input;
layout(location = 1) flat in uint tint_symbol_loc1_Input;
layout(location = 2) in float tint_symbol_loc2_Input;
layout(location = 3) in vec4 tint_symbol_loc3_Input;
void tint_symbol_inner(int loc0, uint loc1, float loc2, vec4 loc3) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  vec4 v = loc3;
}
void main() {
  tint_symbol_inner(tint_symbol_loc0_Input, tint_symbol_loc1_Input, tint_symbol_loc2_Input, tint_symbol_loc3_Input);
}
