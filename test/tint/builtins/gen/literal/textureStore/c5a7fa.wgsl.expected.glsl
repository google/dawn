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

layout(binding = 0, r16ui) uniform highp writeonly uimage3D f_arg_0;
void textureStore_c5a7fa() {
  imageStore(f_arg_0, ivec3(uvec3(1u)), uvec4(1u));
}
void main() {
  textureStore_c5a7fa();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, r16ui) uniform highp writeonly uimage3D arg_0;
void textureStore_c5a7fa() {
  imageStore(arg_0, ivec3(uvec3(1u)), uvec4(1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_c5a7fa();
}

tint executable returned error: exit status 1
