SKIP: FAILED

#version 310 es

struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};
precision highp float;
precision highp int;


Interface main() {
  return Interface(0, 0u, ivec4(0), uvec4(0u), vec4(0.0f));
}
int main(Interface inputs) {
  return inputs.i;
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'structure' :  entry point cannot return a value
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};
precision highp float;
precision highp int;


Interface main() {
  return Interface(0, 0u, ivec4(0), uvec4(0u), vec4(0.0f));
}
int main(Interface inputs) {
  return inputs.i;
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
