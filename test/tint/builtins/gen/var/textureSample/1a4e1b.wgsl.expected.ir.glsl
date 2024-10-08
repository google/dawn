#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSample_1a4e1b() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  vec2 v_1 = arg_2;
  float res = texture(arg_0_arg_1, vec4(v_1, float(arg_3), 0.0f));
  return res;
}
void main() {
  v.tint_symbol = textureSample_1a4e1b();
}
