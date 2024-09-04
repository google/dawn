SKIP: FAILED

#version 310 es

uvec3 x_1 = uvec3(0u);
void main_1() {
  uvec3 x_2 = x_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(uvec3 x_1_param) {
  x_1 = x_1_param;
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'main' : function cannot take any parameter(s) 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
