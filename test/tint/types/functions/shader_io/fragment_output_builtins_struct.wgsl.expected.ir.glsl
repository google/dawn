SKIP: FAILED

#version 310 es

struct FragmentOutputs {
  float frag_depth;
  uint sample_mask;
};

FragmentOutputs main() {
  return FragmentOutputs(1.0f, 1u);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
