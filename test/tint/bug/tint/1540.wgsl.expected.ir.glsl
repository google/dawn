SKIP: FAILED

#version 310 es


struct S {
  bool e;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool b = false;
  S v = S((true & b));
}
error: Error parsing GLSL shader:
ERROR: 0:11: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' const bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
