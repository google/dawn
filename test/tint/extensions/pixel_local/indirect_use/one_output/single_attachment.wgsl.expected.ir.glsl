SKIP: INVALID

#version 310 es

struct PixelLocal {
  uint a;
};
precision highp float;
precision highp int;


PixelLocal P;
void f0() {
  P.a = (P.a + 9u);
}
void f1() {
  f0();
  P.a = (P.a + 8u);
}
void f2() {
  P.a = (P.a + 7u);
  f1();
}
vec4 main() {
  f2();
  return vec4(2.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'float' :  entry point cannot return a value
ERROR: 0:22: '' : compilation terminated
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
