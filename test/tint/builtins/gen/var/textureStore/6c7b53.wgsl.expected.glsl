SKIP: FAILED

Error parsing GLSL shader:
ERROR: 0:5: 'image load-store format' : not supported with this profile: es
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



Error parsing GLSL shader:
ERROR: 0:3: 'image load-store format' : not supported with this profile: es
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rg8i) uniform highp writeonly iimage3D f_arg_0;
void textureStore_6c7b53() {
  ivec3 arg_1 = ivec3(1);
  ivec4 arg_2 = ivec4(1);
  imageStore(f_arg_0, arg_1, arg_2);
}
void main() {
  textureStore_6c7b53();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rg8i) uniform highp writeonly iimage3D arg_0;
void textureStore_6c7b53() {
  ivec3 arg_1 = ivec3(1);
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, arg_1, arg_2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_6c7b53();
}

tint executable returned error: exit status 1
