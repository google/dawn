#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec2 tint_symbol;
} v;
vec2 fwidth_b83ebb() {
  vec2 res = fwidth(vec2(1.0f));
  return res;
}
void main() {
  v.tint_symbol = fwidth_b83ebb();
}
