#version 310 es
precision highp float;
precision highp int;

layout(location = 2) in vec2 vUv_1;
layout(location = 0) out vec4 color_1;
struct FragmentInput {
  vec2 vUv;
};

struct FragmentOutput {
  vec4 color;
};

uniform highp sampler2DShadow depthMap_texSampler;

FragmentOutput tint_symbol(FragmentInput fIn) {
  float tint_symbol_1 = texture(depthMap_texSampler, vec3(fIn.vUv, 0.0f));
  vec3 color = vec3(tint_symbol_1, tint_symbol_1, tint_symbol_1);
  FragmentOutput fOut = FragmentOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  fOut.color = vec4(color, 1.0f);
  return fOut;
}

void main() {
  FragmentInput tint_symbol_2 = FragmentInput(vUv_1);
  FragmentOutput inner_result = tint_symbol(tint_symbol_2);
  color_1 = inner_result.color;
  return;
}
