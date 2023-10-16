SKIP: FAILED

#version 310 es

layout(rgba8i) uniform highp writeonly iimage2DArray arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureLoad_32a7b8() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  ivec4 res = imageLoad(arg_0, ivec3(uvec3(arg_1, uint(arg_2))));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureLoad_32a7b8();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'writeonly' : argument cannot drop memory qualifier when passed to formal parameter 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;

layout(rgba8i) uniform highp writeonly iimage2DArray arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureLoad_32a7b8() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  ivec4 res = imageLoad(arg_0, ivec3(uvec3(arg_1, uint(arg_2))));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureLoad_32a7b8();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'writeonly' : argument cannot drop memory qualifier when passed to formal parameter 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(rgba8i) uniform highp writeonly iimage2DArray arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureLoad_32a7b8() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  ivec4 res = imageLoad(arg_0, ivec3(uvec3(arg_1, uint(arg_2))));
  prevent_dce.inner = res;
}

void compute_main() {
  textureLoad_32a7b8();
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



