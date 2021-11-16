#version 310 es
precision mediump float;

struct FragmentInput {
  vec2 vUv;
};
struct FragmentOutput {
  vec4 color;
};

uniform highp sampler2D depthMap;


struct tint_symbol_3 {
  vec2 vUv;
};
struct tint_symbol_4 {
  vec4 color;
};

FragmentOutput tint_symbol_inner(FragmentInput fIn) {
  float tint_symbol_1 = texture(depthMap, fIn.vUv).x;
  vec3 color = vec3(tint_symbol_1, tint_symbol_1, tint_symbol_1);
  FragmentOutput fOut = FragmentOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  fOut.color = vec4(color, 1.0f);
  return fOut;
}

tint_symbol_4 tint_symbol(tint_symbol_3 tint_symbol_2) {
  FragmentInput tint_symbol_5 = FragmentInput(tint_symbol_2.vUv);
  FragmentOutput inner_result = tint_symbol_inner(tint_symbol_5);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.color = inner_result.color;
  return wrapper_result;
}
in vec2 vUv;
out vec4 color;
void main() {
  tint_symbol_3 inputs;
  inputs.vUv = vUv;
  tint_symbol_4 outputs;
  outputs = tint_symbol(inputs);
  color = outputs.color;
}


