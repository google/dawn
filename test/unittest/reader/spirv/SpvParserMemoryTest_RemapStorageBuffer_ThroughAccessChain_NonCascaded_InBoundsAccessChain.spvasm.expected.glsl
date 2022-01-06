SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  uint field0;
  uint field1[];
};

layout (binding = 0) buffer S_1 {
  uint field0;
  uint field1[];
} myvar;

void main_1() {
  myvar.field0 = 0u;
  myvar.field1[1u] = 0u;
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



