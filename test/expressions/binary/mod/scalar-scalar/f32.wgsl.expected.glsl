SKIP: FAILED

#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f() {
  float r = (1.0f % 2.0f);
  return;
}
void main() {
  f();
}


Error parsing GLSL shader:
ERROR: 0:6: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' const float' and a right operand of type ' const float' (or there is no acceptable conversion)
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



