#version 310 es
precision highp float;
precision highp int;


struct tint_push_constant_struct {
  float tint_frag_depth_min;
  float tint_frag_depth_max;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
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
