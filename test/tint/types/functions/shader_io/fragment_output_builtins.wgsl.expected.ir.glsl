SKIP: FAILED

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
  gl_SampleMask = main2_inner();
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'assign' :  cannot convert from ' global highp uint' to ' out unsized 1-element array of highp int SampleMaskIn'
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
