#version 310 es
precision mediump float;

struct In {
  float none;
  float tint_symbol;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
};
struct tint_symbol_4 {
  float none;
  float tint_symbol;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
};

void tint_symbol_1_inner(In tint_symbol_2) {
}

void tint_symbol_1(tint_symbol_4 tint_symbol_3) {
  In tint_symbol_5 = In(tint_symbol_3.none, tint_symbol_3.tint_symbol, tint_symbol_3.perspective_center, tint_symbol_3.perspective_centroid, tint_symbol_3.perspective_sample, tint_symbol_3.linear_center, tint_symbol_3.linear_centroid, tint_symbol_3.linear_sample);
  tint_symbol_1_inner(tint_symbol_5);
  return;
}
in float none;
in float tint_symbol;
in float perspective_center;
in float perspective_centroid;
in float perspective_sample;
in float linear_center;
in float linear_centroid;
in float linear_sample;
void main() {
  tint_symbol_4 inputs;
  inputs.none = none;
  inputs.tint_symbol = tint_symbol;
  inputs.perspective_center = perspective_center;
  inputs.perspective_centroid = perspective_centroid;
  inputs.perspective_sample = perspective_sample;
  inputs.linear_center = linear_center;
  inputs.linear_centroid = linear_centroid;
  inputs.linear_sample = linear_sample;
  tint_symbol_1(inputs);
}


