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

layout(binding = 0, rg16) uniform highp writeonly image3D f_arg_0;
void textureStore_a3652b() {
  uvec3 arg_1 = uvec3(1u);
  vec4 arg_2 = vec4(1.0f);
  vec4 v = arg_2;
  imageStore(f_arg_0, ivec3(arg_1), v);
}
void main() {
  textureStore_a3652b();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rg16) uniform highp writeonly image3D arg_0;
void textureStore_a3652b() {
  uvec3 arg_1 = uvec3(1u);
  vec4 arg_2 = vec4(1.0f);
  vec4 v = arg_2;
  imageStore(arg_0, ivec3(arg_1), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_a3652b();
}

tint executable returned error: exit status 1
