SKIP: INVALID

#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32ui) uniform highp uimage2DArray arg_0;
void textureStore_bd6602() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  uvec4 arg_3 = uvec4(1u);
  uvec2 v = arg_1;
  uvec4 v_1 = arg_3;
  imageStore(arg_0, uvec3(v, uint(arg_2)), v_1);
}
void main() {
  textureStore_bd6602();
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'imageStore' : no matching overloaded function found 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 460

layout(binding = 0, rg32ui) uniform highp uimage2DArray arg_0;
void textureStore_bd6602() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  uvec4 arg_3 = uvec4(1u);
  uvec2 v = arg_1;
  uvec4 v_1 = arg_3;
  imageStore(arg_0, uvec3(v, uint(arg_2)), v_1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_bd6602();
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'imageStore' : no matching overloaded function found 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
