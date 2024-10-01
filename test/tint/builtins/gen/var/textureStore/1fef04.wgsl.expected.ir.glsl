SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32i) uniform highp iimage1D arg_0;
void textureStore_1fef04() {
  uint arg_1 = 1u;
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, arg_1, arg_2);
}
void main() {
  textureStore_1fef04();
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'iimage1D' : Reserved word. 
WARNING: 0:5: 'layout' : useless application of layout qualifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, r32i) uniform highp iimage1D arg_0;
void textureStore_1fef04() {
  uint arg_1 = 1u;
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, arg_1, arg_2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_1fef04();
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'iimage1D' : Reserved word. 
WARNING: 0:3: 'layout' : useless application of layout qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
