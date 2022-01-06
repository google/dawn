SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  float field0;
  float age[];
};

layout (binding = 0) buffer S_1 {
  float field0;
  float age[];
} myvar;

void main_1() {
  myvar.age[2u] = 42.0f;
  return;
}

void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:6: '' : array size required 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



