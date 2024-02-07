#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in float none_1;
layout(location = 1) flat in float tint_symbol_3;
layout(location = 2) in float perspective_center_1;
layout(location = 3) centroid in float perspective_centroid_1;
layout(location = 4) in float perspective_sample_1;
layout(location = 5) in float linear_center_1;
layout(location = 6) centroid in float linear_centroid_1;
layout(location = 7) in float linear_sample_1;
layout(location = 8) in float perspective_default_1;
layout(location = 9) in float linear_default_1;
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

void tint_symbol_1(In tint_symbol_2) {
}

void main() {
  In tint_symbol_4 = In(none_1, tint_symbol_3, perspective_center_1, perspective_centroid_1, perspective_sample_1, linear_center_1, linear_centroid_1, linear_sample_1, perspective_default_1, linear_default_1);
  tint_symbol_1(tint_symbol_4);
  return;
}
