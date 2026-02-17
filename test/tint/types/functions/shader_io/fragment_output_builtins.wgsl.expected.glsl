//
// main1
//
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) uniform uint tint_immediates[2];
float main1_inner() {
  return 1.0f;
}
void main() {
  float v = main1_inner();
  gl_FragDepth = clamp(v, uintBitsToFloat(tint_immediates[0u]), uintBitsToFloat(tint_immediates[1u]));
}
//
// main2
//
#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;

uint main2_inner() {
  return 1u;
}
void main() {
  gl_SampleMask[0u] = int(main2_inner());
}
