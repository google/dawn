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

layout(binding = 0, rg8i) uniform highp writeonly iimage2DArray f_arg_0;
void textureStore_52f047() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec4 arg_3 = ivec4(1);
  ivec2 v = arg_1;
  ivec4 v_1 = arg_3;
  imageStore(f_arg_0, ivec3(v, int(arg_2)), v_1);
}
void main() {
  textureStore_52f047();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rg8i) uniform highp writeonly iimage2DArray arg_0;
void textureStore_52f047() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec4 arg_3 = ivec4(1);
  ivec2 v = arg_1;
  ivec4 v_1 = arg_3;
  imageStore(arg_0, ivec3(v, int(arg_2)), v_1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_52f047();
}

tint executable returned error: exit status 1
