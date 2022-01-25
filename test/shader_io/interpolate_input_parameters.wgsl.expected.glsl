#version 310 es
precision mediump float;

struct tint_symbol_3 {
  float none;
  float tint_symbol_1;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
};

void tint_symbol_inner(float none, float tint_symbol_1, float perspective_center, float perspective_centroid, float perspective_sample, float linear_center, float linear_centroid, float linear_sample) {
}

void tint_symbol(tint_symbol_3 tint_symbol_2) {
  tint_symbol_inner(tint_symbol_2.none, tint_symbol_2.tint_symbol_1, tint_symbol_2.perspective_center, tint_symbol_2.perspective_centroid, tint_symbol_2.perspective_sample, tint_symbol_2.linear_center, tint_symbol_2.linear_centroid, tint_symbol_2.linear_sample);
  return;
}
layout(location = 0) in float none;
layout(location = 1) in float tint_symbol_1;
layout(location = 2) in float perspective_center;
layout(location = 3) in float perspective_centroid;
layout(location = 4) in float perspective_sample;
layout(location = 5) in float linear_center;
layout(location = 6) in float linear_centroid;
layout(location = 7) in float linear_sample;
void main() {
  tint_symbol_3 inputs;
  inputs.none = none;
  inputs.tint_symbol_1 = tint_symbol_1;
  inputs.perspective_center = perspective_center;
  inputs.perspective_centroid = perspective_centroid;
  inputs.perspective_sample = perspective_sample;
  inputs.linear_center = linear_center;
  inputs.linear_centroid = linear_centroid;
  inputs.linear_sample = linear_sample;
  tint_symbol(inputs);
}


