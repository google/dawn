#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp samplerCubeArray arg_0_arg_1;

vec4 textureSample_bc7477() {
  vec3 arg_2 = vec3(1.0f);
  uint arg_3 = 1u;
  vec4 res = texture(arg_0_arg_1, vec4(arg_2, float(arg_3)));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSample_bc7477();
}

void main() {
  fragment_main();
  return;
}
