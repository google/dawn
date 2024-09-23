SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

layout(binding = 0, rgba8i) uniform highp writeonly iimage2DArray arg_0;
ivec4 textureLoad_f2c311() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec4 res = imageLoad(arg_0, ivec3(arg_1, arg_2));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureLoad_f2c311();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'writeonly' : argument cannot drop memory qualifier when passed to formal parameter 
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

layout(binding = 0, rgba8i) uniform highp writeonly iimage2DArray arg_0;
ivec4 textureLoad_f2c311() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec4 res = imageLoad(arg_0, ivec3(arg_1, arg_2));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureLoad_f2c311();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'writeonly' : argument cannot drop memory qualifier when passed to formal parameter 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
