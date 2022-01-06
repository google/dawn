SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  uint first;
  uint rtarr[];
};

layout (binding = 0) buffer S_1 {
  uint first;
  uint rtarr[];
} myvar;

void main_1() {
  uint tint_symbol_2 = 0u;
  myvar.GetDimensions(tint_symbol_2);
  uint tint_symbol_3 = ((tint_symbol_2 - 4u) / 4u);
  uint x_1 = tint_symbol_3;
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



