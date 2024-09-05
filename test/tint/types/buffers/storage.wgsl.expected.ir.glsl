#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  float tint_symbol_1[];
} v;
void main() {
  float a = v.tint_symbol_1[0];
}
