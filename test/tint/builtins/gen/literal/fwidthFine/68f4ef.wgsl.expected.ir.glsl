#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
vec4 fwidthFine_68f4ef() {
  vec4 res = fwidth(vec4(1.0f));
  return res;
}
void main() {
  v.tint_symbol = fwidthFine_68f4ef();
}
