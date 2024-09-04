SKIP: FAILED

#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bvec3 a = bvec3(true, true, false);
  bvec3 b = bvec3(true, false, true);
  bvec3 r = (a & b);
}
error: Error parsing GLSL shader:
ERROR: 0:7: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp 3-component vector of bool' and a right operand of type ' temp 3-component vector of bool' (or there is no acceptable conversion)
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
