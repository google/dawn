#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec2 tint_symbol;
} v;
vec2 dpdxFine_9631de() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdx(arg_0);
  return res;
}
void main() {
  v.tint_symbol = dpdxFine_9631de();
}
