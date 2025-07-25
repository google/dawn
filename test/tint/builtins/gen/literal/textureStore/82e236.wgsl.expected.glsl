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

layout(binding = 0, r8ui) uniform highp writeonly uimage2DArray f_arg_0;
void textureStore_82e236() {
  imageStore(f_arg_0, ivec3(ivec2(1), int(1u)), uvec4(1u));
}
void main() {
  textureStore_82e236();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, r8ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_82e236() {
  imageStore(arg_0, ivec3(ivec2(1), int(1u)), uvec4(1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_82e236();
}

tint executable returned error: exit status 1
