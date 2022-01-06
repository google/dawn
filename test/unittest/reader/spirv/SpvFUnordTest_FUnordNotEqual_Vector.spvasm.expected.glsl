SKIP: FAILED

#version 310 es
precision mediump float;

void main_1() {
  bvec2 x_1 = !(equal(vec2(50.0f, 60.0f), vec2(60.0f, 50.0f)));
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
ERROR: 0:5: '!' :  wrong operand type no operation '!' exists that takes an operand of type  const 2-component vector of bool (or there is no acceptable conversion)
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



