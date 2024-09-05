#version 310 es
precision highp float;
precision highp int;


struct FragmentOutputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
};

layout(location = 0) out int tint_symbol_loc0_Output;
layout(location = 1) out uint tint_symbol_loc1_Output;
layout(location = 2) out float tint_symbol_loc2_Output;
layout(location = 3) out vec4 tint_symbol_loc3_Output;
FragmentOutputs tint_symbol_inner() {
  return FragmentOutputs(1, 1u, 1.0f, vec4(1.0f, 2.0f, 3.0f, 4.0f));
}
void main() {
  FragmentOutputs v = tint_symbol_inner();
  tint_symbol_loc0_Output = v.loc0;
  tint_symbol_loc1_Output = v.loc1;
  tint_symbol_loc2_Output = v.loc2;
  tint_symbol_loc3_Output = v.loc3;
}
