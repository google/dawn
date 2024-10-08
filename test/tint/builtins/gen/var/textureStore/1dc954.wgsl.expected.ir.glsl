SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba32i) uniform highp writeonly iimage2D arg_0;
void textureStore_1dc954() {
  uint arg_1 = 1u;
  ivec4 arg_2 = ivec4(1);
  ivec4 v = arg_2;
  imageStore(arg_0, uvec2(arg_1, 0u), v);
}
void main() {
  textureStore_1dc954();
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'imageStore' : no matching overloaded function found 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, rgba32i) uniform highp writeonly iimage2D arg_0;
void textureStore_1dc954() {
  uint arg_1 = 1u;
  ivec4 arg_2 = ivec4(1);
  ivec4 v = arg_2;
  imageStore(arg_0, uvec2(arg_1, 0u), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_1dc954();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'imageStore' : no matching overloaded function found 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
