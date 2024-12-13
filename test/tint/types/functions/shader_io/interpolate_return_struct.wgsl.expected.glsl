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

layout(location = 0) out float tint_interstage_location0;
layout(location = 1) flat out float tint_interstage_location1;
layout(location = 2) out float tint_interstage_location2;
layout(location = 3) centroid out float tint_interstage_location3;
layout(location = 4) out float tint_interstage_location4;
layout(location = 5) out float tint_interstage_location5;
layout(location = 6) centroid out float tint_interstage_location6;
layout(location = 7) out float tint_interstage_location7;
Out tint_symbol_1_inner() {
  return Out(vec4(0.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}
void main() {
  Out v = tint_symbol_1_inner();
  gl_Position = v.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_interstage_location0 = v.none;
  tint_interstage_location1 = v.tint_symbol;
  tint_interstage_location2 = v.perspective_center;
  tint_interstage_location3 = v.perspective_centroid;
  tint_interstage_location4 = v.perspective_sample;
  tint_interstage_location5 = v.linear_center;
  tint_interstage_location6 = v.linear_centroid;
  tint_interstage_location7 = v.linear_sample;
  gl_PointSize = 1.0f;
}
