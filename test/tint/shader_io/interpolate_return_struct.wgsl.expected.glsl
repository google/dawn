#version 310 es

layout(location = 0) out float none_1;
layout(location = 1) flat out float tint_symbol_2;
layout(location = 2) out float perspective_center_1;
layout(location = 3) centroid out float perspective_centroid_1;
layout(location = 4) out float perspective_sample_1;
layout(location = 5) out float linear_center_1;
layout(location = 6) centroid out float linear_centroid_1;
layout(location = 7) out float linear_sample_1;
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

Out tint_symbol_1() {
  Out tint_symbol_3 = Out(vec4(0.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  return tint_symbol_3;
}

void main() {
  gl_PointSize = 1.0;
  Out inner_result = tint_symbol_1();
  gl_Position = inner_result.pos;
  none_1 = inner_result.none;
  tint_symbol_2 = inner_result.tint_symbol;
  perspective_center_1 = inner_result.perspective_center;
  perspective_centroid_1 = inner_result.perspective_centroid;
  perspective_sample_1 = inner_result.perspective_sample;
  linear_center_1 = inner_result.linear_center;
  linear_centroid_1 = inner_result.linear_centroid;
  linear_sample_1 = inner_result.linear_sample;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
