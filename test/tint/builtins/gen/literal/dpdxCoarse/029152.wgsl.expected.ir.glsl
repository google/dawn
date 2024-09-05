#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
float dpdxCoarse_029152() {
  float res = dFdx(1.0f);
  return res;
}
void main() {
  v.tint_symbol = dpdxCoarse_029152();
}
