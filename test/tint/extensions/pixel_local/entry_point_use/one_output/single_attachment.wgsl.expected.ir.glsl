SKIP: INVALID

#version 310 es

struct PixelLocal {
  uint a;
};
precision highp float;
precision highp int;


PixelLocal P;
vec4 main() {
  P.a = (P.a + 42u);
  return vec4(2.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'float' :  entry point cannot return a value
ERROR: 0:11: '' : compilation terminated
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
