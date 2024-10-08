#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
uniform highp sampler2DArray arg_0_arg_1;
vec4 textureSample_17e988() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  vec2 v_1 = arg_2;
  vec4 res = textureOffset(arg_0_arg_1, vec3(v_1, float(arg_3)), ivec2(1));
  return res;
}
void main() {
  v.tint_symbol = textureSample_17e988();
}
