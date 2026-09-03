#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
uniform highp sampler2DArray f_arg_0_arg_1;
vec4 textureSampleBias_87915c() {
  vec4 res = textureOffset(f_arg_0_arg_1, vec3(1.0f), ivec2(1), 1.0f);
  return res;
}
void main() {
  v.inner = textureSampleBias_87915c();
}
