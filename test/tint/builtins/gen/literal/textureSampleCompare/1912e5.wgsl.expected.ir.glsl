#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp samplerCubeArrayShadow arg_0_arg_1;
float textureSampleCompare_1912e5() {
  float res = texture(arg_0_arg_1, vec4(vec3(1.0f), float(1u)), 1.0f);
  return res;
}
void main() {
  v.tint_symbol = textureSampleCompare_1912e5();
}
