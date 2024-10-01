SKIP: INVALID

#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32ui) uniform highp uimage3D arg_0;
void textureStore_473ead() {
  uvec3 arg_1 = uvec3(1u);
  uvec4 arg_2 = uvec4(1u);
  imageStore(arg_0, arg_1, arg_2);
}
void main() {
  textureStore_473ead();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'imageStore' : no matching overloaded function found 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 460

layout(binding = 0, rg32ui) uniform highp uimage3D arg_0;
void textureStore_473ead() {
  uvec3 arg_1 = uvec3(1u);
  uvec4 arg_2 = uvec4(1u);
  imageStore(arg_0, arg_1, arg_2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_473ead();
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'imageStore' : no matching overloaded function found 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
