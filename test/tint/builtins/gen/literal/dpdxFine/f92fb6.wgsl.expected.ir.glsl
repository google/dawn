#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec3 tint_symbol;
} v;
vec3 dpdxFine_f92fb6() {
  vec3 res = dFdx(vec3(1.0f));
  return res;
}
void main() {
  v.tint_symbol = dpdxFine_f92fb6();
}
