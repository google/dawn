SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8_snorm) uniform highp writeonly image1D arg_0;
void textureStore_959d94() {
  imageStore(arg_0, 1u, vec4(1.0f));
}
void main() {
  textureStore_959d94();
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'image1D' : Reserved word. 
WARNING: 0:5: 'layout' : useless application of layout qualifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, rgba8_snorm) uniform highp writeonly image1D arg_0;
void textureStore_959d94() {
  imageStore(arg_0, 1u, vec4(1.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_959d94();
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'image1D' : Reserved word. 
WARNING: 0:3: 'layout' : useless application of layout qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
