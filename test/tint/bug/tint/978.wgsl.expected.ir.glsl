#version 310 es
precision highp float;
precision highp int;


struct FragmentOutput {
  vec4 color;
};

struct FragmentInput {
  vec2 vUv;
};

uniform highp sampler2DShadow depthMap_texSampler;
layout(location = 2) in vec2 tint_symbol_loc2_Input;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
FragmentOutput tint_symbol_inner(FragmentInput fIn) {
  float tint_symbol_1 = texture(depthMap_texSampler, vec3(fIn.vUv, 0.0f));
  vec3 color = vec3(tint_symbol_1, tint_symbol_1, tint_symbol_1);
  FragmentOutput fOut = FragmentOutput(vec4(0.0f));
  fOut.color = vec4(color, 1.0f);
  return fOut;
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner(FragmentInput(tint_symbol_loc2_Input)).color;
}
