#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSample_60bf45() {
  vec4 v_1 = vec4(vec2(1.0f), float(1), 0.0f);
  vec2 v_2 = dFdx(vec2(1.0f));
  float res = textureGradOffset(arg_0_arg_1, v_1, v_2, dFdy(vec2(1.0f)), ivec2(1));
  return res;
}
void main() {
  v.tint_symbol = textureSample_60bf45();
}
