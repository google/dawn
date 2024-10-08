#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DShadow arg_0_arg_1;
float textureSample_38bbb9() {
  vec2 arg_2 = vec2(1.0f);
  float res = texture(arg_0_arg_1, vec3(arg_2, 0.0f));
  return res;
}
void main() {
  v.tint_symbol = textureSample_38bbb9();
}
