#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
uniform highp sampler2DArray arg_0_arg_1;
vec4 textureSampleBias_87915c() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  vec2 v_1 = arg_2;
  float v_2 = arg_4;
  vec4 res = textureOffset(arg_0_arg_1, vec3(v_1, float(arg_3)), ivec2(1), v_2);
  return res;
}
void main() {
  v.tint_symbol = textureSampleBias_87915c();
}
