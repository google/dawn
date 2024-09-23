SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

layout(binding = 0, rgba16i) uniform highp writeonly iimage2D arg_0;
ivec4 textureLoad_d8be5a() {
  ivec4 res = imageLoad(arg_0, ivec2(1, 0));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureLoad_d8be5a();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'writeonly' : argument cannot drop memory qualifier when passed to formal parameter 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

layout(binding = 0, rgba16i) uniform highp writeonly iimage2D arg_0;
ivec4 textureLoad_d8be5a() {
  ivec4 res = imageLoad(arg_0, ivec2(1, 0));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureLoad_d8be5a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'writeonly' : argument cannot drop memory qualifier when passed to formal parameter 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
