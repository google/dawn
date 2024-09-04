SKIP: FAILED

#version 310 es

struct S {
  float field0;
  float age;
};

struct S_1 {
  float field0;
  float ancientness;
};
precision highp float;
precision highp int;


S myvar = S(0.0f, 0.0f);
S_1 myvar2 = S_1(0.0f, 0.0f);
void main_1() {
  myvar.age = 42.0f;
  myvar2.ancientness = 420.0f;
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
