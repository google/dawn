SKIP: FAILED

#version 310 es
precision mediump float;

void main_1() {
  float x_1 = (50.0f % 60.0f);
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
ERROR: 0:5: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' const float' and a right operand of type ' const float' (or there is no acceptable conversion)
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



