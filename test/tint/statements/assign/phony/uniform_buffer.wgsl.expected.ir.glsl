SKIP: FAILED

#version 310 es

struct S {
  int i;
};

uniform S u = S(0);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'uniform' :  cannot initialize this type of qualifier  
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1
