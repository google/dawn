#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSample_1a4e1b() {
  float res = texture(arg_0_arg_1, vec4(vec2(1.0f), float(1u), 0.0f));
  return res;
}
void main() {
  v.tint_symbol = textureSample_1a4e1b();
}
