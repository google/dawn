SKIP: FAILED

#version 310 es

struct S {
  vec3 v;
  uint i;
};

S io;
vec3 Bad(uint index, vec3 rd) {
  vec3 normal = vec3(0.0f);
  normal[index] = -(sign(rd[index]));
  return normalize(normal);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(uint idx) {
  io.v = Bad(io.i, io.v);
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'main' : function cannot take any parameter(s) 
ERROR: 0:15: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
