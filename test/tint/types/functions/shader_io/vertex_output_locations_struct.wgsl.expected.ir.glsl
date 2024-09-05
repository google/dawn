#version 310 es


struct VertexOutputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  vec4 position;
};

layout(location = 0) flat out int tint_symbol_loc0_Output;
layout(location = 1) flat out uint tint_symbol_loc1_Output;
layout(location = 2) out float tint_symbol_loc2_Output;
layout(location = 3) out vec4 tint_symbol_loc3_Output;
VertexOutputs tint_symbol_inner() {
  return VertexOutputs(1, 1u, 1.0f, vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(0.0f));
}
void main() {
  VertexOutputs v = tint_symbol_inner();
  tint_symbol_loc0_Output = v.loc0;
  tint_symbol_loc1_Output = v.loc1;
  tint_symbol_loc2_Output = v.loc2;
  tint_symbol_loc3_Output = v.loc3;
  gl_Position = v.position;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
