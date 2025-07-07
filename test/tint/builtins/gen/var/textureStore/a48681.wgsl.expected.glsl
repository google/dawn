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

layout(binding = 0, rg8_snorm) uniform highp writeonly image2DArray f_arg_0;
void textureStore_a48681() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  vec4 arg_3 = vec4(1.0f);
  vec4 v = arg_3;
  imageStore(f_arg_0, ivec3(arg_1, arg_2), v);
}
void main() {
  textureStore_a48681();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rg8_snorm) uniform highp writeonly image2DArray arg_0;
void textureStore_a48681() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  vec4 arg_3 = vec4(1.0f);
  vec4 v = arg_3;
  imageStore(arg_0, ivec3(arg_1, arg_2), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_a48681();
}

tint executable returned error: exit status 1
