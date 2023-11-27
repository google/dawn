#version 310 es
precision highp float;

float main1() {
  return 1.0f;
}

void main() {
  float inner_result = main1();
  gl_FragDepth = inner_result;
  return;
}
#version 310 es
#extension GL_OES_sample_variables : require
precision highp float;

uint main2() {
  return 1u;
}

void main() {
  uint inner_result = main2();
  gl_SampleMask[0] = int(inner_result);
  return;
}
