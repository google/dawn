SKIP: FAILED

#version 310 es

int data[];
int foo() {
  return data[0];
}
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
  foo();
}
error: Error parsing GLSL shader:
ERROR: 0:3: '' : array size required 
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1
