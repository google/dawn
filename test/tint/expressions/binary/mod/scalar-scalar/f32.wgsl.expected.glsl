SKIP: FAILED

#version 310 es

void f() {
  float r = (1.0f % 2.0f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' const float' and a right operand of type ' const float' (or there is no acceptable conversion)
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



