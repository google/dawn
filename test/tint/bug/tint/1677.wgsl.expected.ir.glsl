SKIP: FAILED

#version 310 es

struct Input {
  ivec3 position;
};

Input tint_symbol;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(uvec3 id) {
  ivec3 pos = (tint_symbol.position - ivec3(0));
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'main' : function cannot take any parameter(s) 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
