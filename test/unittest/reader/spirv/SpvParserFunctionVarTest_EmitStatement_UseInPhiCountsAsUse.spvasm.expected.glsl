SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  uint field0;
  float field1;
  uint field2[2];
};

void main_1() {
  bool x_101_phi = false;
  bool x_11 = (true & true);
  bool x_12 = !(x_11);
  x_101_phi = x_11;
  if (true) {
    x_101_phi = x_12;
  }
  bool x_101 = x_101_phi;
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
ERROR: 0:12: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' const bool' and a right operand of type ' const bool' (or there is no acceptable conversion)
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



