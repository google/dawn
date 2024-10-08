#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DShadow arg_0_arg_1;
float textureSample_0dff6c() {
  vec2 arg_2 = vec2(1.0f);
  float res = textureOffset(arg_0_arg_1, vec3(arg_2, 0.0f), ivec2(1));
  return res;
}
void main() {
  v.tint_symbol = textureSample_0dff6c();
}
