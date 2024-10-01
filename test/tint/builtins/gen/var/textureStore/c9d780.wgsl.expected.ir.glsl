SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8) uniform highp writeonly image1D arg_0;
void textureStore_c9d780() {
  uint arg_1 = 1u;
  vec4 arg_2 = vec4(1.0f);
  imageStore(arg_0, arg_1, arg_2.zyxw);
}
void main() {
  textureStore_c9d780();
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'image1D' : Reserved word. 
WARNING: 0:5: 'layout' : useless application of layout qualifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, rgba8) uniform highp writeonly image1D arg_0;
void textureStore_c9d780() {
  uint arg_1 = 1u;
  vec4 arg_2 = vec4(1.0f);
  imageStore(arg_0, arg_1, arg_2.zyxw);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_c9d780();
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'image1D' : Reserved word. 
WARNING: 0:3: 'layout' : useless application of layout qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
