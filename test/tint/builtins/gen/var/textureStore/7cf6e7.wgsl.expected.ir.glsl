SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba16f) uniform highp writeonly image2D arg_0;
void textureStore_7cf6e7() {
  uvec2 arg_1 = uvec2(1u);
  vec4 arg_2 = vec4(1.0f);
  imageStore(arg_0, arg_1, arg_2);
}
void main() {
  textureStore_7cf6e7();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'imageStore' : no matching overloaded function found 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, rgba16f) uniform highp writeonly image2D arg_0;
void textureStore_7cf6e7() {
  uvec2 arg_1 = uvec2(1u);
  vec4 arg_2 = vec4(1.0f);
  imageStore(arg_0, arg_1, arg_2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_7cf6e7();
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'imageStore' : no matching overloaded function found 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
