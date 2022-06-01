#version 310 es

layout(location = 0) in int loc0_1;
layout(location = 1) in uint loc1_1;
layout(location = 2) in float loc2_1;
layout(location = 3) in vec4 loc3_1;
struct VertexInputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
};

vec4 tint_symbol(VertexInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  vec4 v = inputs.loc3;
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  VertexInputs tint_symbol_1 = VertexInputs(loc0_1, loc1_1, loc2_1, loc3_1);
  vec4 inner_result = tint_symbol(tint_symbol_1);
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
