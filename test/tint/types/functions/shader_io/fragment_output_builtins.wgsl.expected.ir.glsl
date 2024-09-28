#version 310 es
precision highp float;
precision highp int;

float main1_inner() {
  return 1.0f;
}
void main() {
  gl_FragDepth = main1_inner();
}
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
