SKIP: FAILED

#version 310 es

struct Out {
  vec4 x;
  vec4 y;
  vec4 z;
};
precision highp float;
precision highp int;


Out main(vec4 fbf) {
  return Out(vec4(10.0f), fbf, vec4(30.0f));
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
