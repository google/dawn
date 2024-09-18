#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp samplerCubeArray arg_0_arg_1;

vec4 textureSample_bc7477() {
  vec4 res = texture(arg_0_arg_1, vec4(vec3(1.0f), float(1u)));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSample_bc7477();
}

void main() {
  fragment_main();
  return;
}
