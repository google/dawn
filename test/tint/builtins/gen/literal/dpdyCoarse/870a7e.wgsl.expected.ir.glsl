#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
float dpdyCoarse_870a7e() {
  float res = dFdy(1.0f);
  return res;
}
void main() {
  v.tint_symbol = dpdyCoarse_870a7e();
}
