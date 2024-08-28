SKIP: FAILED

#version 310 es

struct S {
  float field0;
  float field1;
};

S x_1 = S(0.0f, 0.0f);
void main_1() {
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
