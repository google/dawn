#version 310 es


struct Out {
  vec4 pos;
  float none;
  float tint_symbol;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
};

layout(location = 0) out float tint_symbol_1_loc0_Output;
layout(location = 1) flat out float tint_symbol_1_loc1_Output;
layout(location = 2) out float tint_symbol_1_loc2_Output;
layout(location = 3) centroid out float tint_symbol_1_loc3_Output;
layout(location = 4) out float tint_symbol_1_loc4_Output;
layout(location = 5) out float tint_symbol_1_loc5_Output;
layout(location = 6) centroid out float tint_symbol_1_loc6_Output;
layout(location = 7) out float tint_symbol_1_loc7_Output;
Out tint_symbol_1_inner() {
  return Out(vec4(0.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}
void main() {
  Out v = tint_symbol_1_inner();
  gl_Position = v.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_symbol_1_loc0_Output = v.none;
  tint_symbol_1_loc1_Output = v.tint_symbol;
  tint_symbol_1_loc2_Output = v.perspective_center;
  tint_symbol_1_loc3_Output = v.perspective_centroid;
  tint_symbol_1_loc4_Output = v.perspective_sample;
  tint_symbol_1_loc5_Output = v.linear_center;
  tint_symbol_1_loc6_Output = v.linear_centroid;
  tint_symbol_1_loc7_Output = v.linear_sample;
  gl_PointSize = 1.0f;
}
