SKIP: FAILED

#version 310 es

struct Out {
  vec4 pos;
};

Out main() {
  return Out(vec4(0.0f));
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'structure' :  entry point cannot return a value
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
