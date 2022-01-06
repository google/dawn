SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  uint field0;
  uint field1[];
};

void main_1() {
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



