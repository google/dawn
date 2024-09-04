SKIP: FAILED

#version 310 es

struct PixelLocal {
  uint a;
};

struct Out {
  vec4 x;
  vec4 y;
  vec4 z;
};
precision highp float;
precision highp int;


PixelLocal P;
Out main() {
  P.a = (P.a + 42u);
  return Out(vec4(10.0f), vec4(20.0f), vec4(30.0f));
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
