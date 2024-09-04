SKIP: FAILED

#version 310 es

struct FragmentOutputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
};
precision highp float;
precision highp int;


FragmentOutputs main() {
  return FragmentOutputs(1, 1u, 1.0f, vec4(1.0f, 2.0f, 3.0f, 4.0f));
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
