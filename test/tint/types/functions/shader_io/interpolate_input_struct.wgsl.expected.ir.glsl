#version 310 es
precision highp float;
precision highp int;


struct In {
  float none;
  float tint_symbol;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
  float perspective_default;
  float linear_default;
};

layout(location = 0) in float tint_symbol_1_loc0_Input;
layout(location = 1) flat in float tint_symbol_1_loc1_Input;
layout(location = 2) in float tint_symbol_1_loc2_Input;
layout(location = 3) centroid in float tint_symbol_1_loc3_Input;
layout(location = 4) in float tint_symbol_1_loc4_Input;
layout(location = 5) in float tint_symbol_1_loc5_Input;
layout(location = 6) centroid in float tint_symbol_1_loc6_Input;
layout(location = 7) in float tint_symbol_1_loc7_Input;
layout(location = 8) in float tint_symbol_1_loc8_Input;
layout(location = 9) in float tint_symbol_1_loc9_Input;
void tint_symbol_1_inner(In tint_symbol_2) {
}
void main() {
  tint_symbol_1_inner(In(tint_symbol_1_loc0_Input, tint_symbol_1_loc1_Input, tint_symbol_1_loc2_Input, tint_symbol_1_loc3_Input, tint_symbol_1_loc4_Input, tint_symbol_1_loc5_Input, tint_symbol_1_loc6_Input, tint_symbol_1_loc7_Input, tint_symbol_1_loc8_Input, tint_symbol_1_loc9_Input));
}
