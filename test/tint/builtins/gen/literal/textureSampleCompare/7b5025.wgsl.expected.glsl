#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow f_arg_0_arg_1;
float textureSampleCompare_7b5025() {
  float v_1 = float(1u);
  vec2 v_2 = dFdx(vec2(1.0f));
  float res = textureGradOffset(f_arg_0_arg_1, vec4(1.0f), v_2, dFdy(vec2(1.0f)), ivec2(1));
  return res;
}
void main() {
  v.inner = textureSampleCompare_7b5025();
}
