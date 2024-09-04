SKIP: FAILED

#version 310 es

struct S {
  int arr[];
};

S s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
error: Error parsing GLSL shader:
ERROR: 0:4: '' : array size required 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
