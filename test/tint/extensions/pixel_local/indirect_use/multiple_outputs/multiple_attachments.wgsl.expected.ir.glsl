SKIP: FAILED

#version 310 es

struct PixelLocal {
  uint a;
  int b;
  float c;
};

struct Out {
  vec4 x;
  vec4 y;
  vec4 z;
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
Out main() {
  f2();
  return Out(vec4(10.0f), vec4(20.0f), vec4(30.0f));
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
