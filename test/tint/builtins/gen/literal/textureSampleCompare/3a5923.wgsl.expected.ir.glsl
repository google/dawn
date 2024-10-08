#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DShadow arg_0_arg_1;
float textureSampleCompare_3a5923() {
  float res = texture(arg_0_arg_1, vec3(vec2(1.0f), 1.0f));
  return res;
}
void main() {
  v.tint_symbol = textureSampleCompare_3a5923();
}
