#version 310 es
precision mediump float;

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
struct tint_symbol_2 {
  float none;
  float tint_symbol;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
  vec4 pos;
};

Out tint_symbol_1_inner() {
  Out tint_symbol_3 = Out(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  return tint_symbol_3;
}

tint_symbol_2 tint_symbol_1() {
  Out inner_result = tint_symbol_1_inner();
  tint_symbol_2 wrapper_result = tint_symbol_2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.pos = inner_result.pos;
  wrapper_result.none = inner_result.none;
  wrapper_result.tint_symbol = inner_result.tint_symbol;
  wrapper_result.perspective_center = inner_result.perspective_center;
  wrapper_result.perspective_centroid = inner_result.perspective_centroid;
  wrapper_result.perspective_sample = inner_result.perspective_sample;
  wrapper_result.linear_center = inner_result.linear_center;
  wrapper_result.linear_centroid = inner_result.linear_centroid;
  wrapper_result.linear_sample = inner_result.linear_sample;
  return wrapper_result;
}
layout(location = 0) out float none;
layout(location = 1) flat out float tint_symbol;
layout(location = 2) out float perspective_center;
layout(location = 3) centroid out float perspective_centroid;
layout(location = 4) out float perspective_sample;
layout(location = 5) out float linear_center;
layout(location = 6) centroid out float linear_centroid;
layout(location = 7) out float linear_sample;
void main() {
  tint_symbol_2 outputs;
  outputs = tint_symbol_1();
  none = outputs.none;
  tint_symbol = outputs.tint_symbol;
  perspective_center = outputs.perspective_center;
  perspective_centroid = outputs.perspective_centroid;
  perspective_sample = outputs.perspective_sample;
  linear_center = outputs.linear_center;
  linear_centroid = outputs.linear_centroid;
  linear_sample = outputs.linear_sample;
  gl_Position = outputs.pos;
  gl_Position.z = 2.0 * gl_Position.z - gl_Position.w;
  gl_Position.y = -gl_Position.y;
}


