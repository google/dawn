#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow f_arg_0_arg_1;
float textureSample_1a4e1b() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  vec2 v_1 = arg_2;
  float res = texture(f_arg_0_arg_1, vec4(v_1, float(arg_3), 0.0f));
  return res;
}
void main() {
  v.inner = textureSample_1a4e1b();
}
