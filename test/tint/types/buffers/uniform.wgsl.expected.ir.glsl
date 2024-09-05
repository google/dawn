#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  vec2 tint_symbol_1;
} v;
void main() {
  float a = v.tint_symbol_1.x;
}
