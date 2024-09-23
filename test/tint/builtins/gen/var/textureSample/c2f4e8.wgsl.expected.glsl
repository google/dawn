#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

uniform highp samplerCubeArrayShadow arg_0_arg_1;

float textureSample_c2f4e8() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  float res = texture(arg_0_arg_1, vec4(arg_2, float(arg_3)), 0.0f);
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSample_c2f4e8();
}

void main() {
  fragment_main();
  return;
}
