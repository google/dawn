#version 310 es
precision highp float;
precision highp int;

struct PushConstants {
  float min_depth;
  float max_depth;
};

layout(location=0) uniform PushConstants push_constants;
float clamp_frag_depth(float v) {
  return clamp(v, push_constants.min_depth, push_constants.max_depth);
}

float main1() {
  return clamp_frag_depth(1.0f);
}

void main() {
  float inner_result = main1();
  gl_FragDepth = inner_result;
  return;
}
#version 310 es
#extension GL_OES_sample_variables : require
precision highp float;
precision highp int;

uint main2() {
  return 1u;
}

void main() {
  uint inner_result = main2();
  gl_SampleMask[0] = int(inner_result);
  return;
}
