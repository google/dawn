SKIP: FAILED

#version 310 es

layout(rgba8) uniform highp writeonly image2D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_8fc29b() {
  vec4 res = imageLoad(arg_0, ivec2(1, 0));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureLoad_8fc29b();
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
ERROR: 0:9: 'writeonly' : argument cannot drop memory qualifier when passed to formal parameter 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;

layout(rgba8) uniform highp writeonly image2D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_8fc29b() {
  vec4 res = imageLoad(arg_0, ivec2(1, 0));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureLoad_8fc29b();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'writeonly' : argument cannot drop memory qualifier when passed to formal parameter 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(rgba8) uniform highp writeonly image2D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_8fc29b() {
  vec4 res = imageLoad(arg_0, ivec2(1, 0));
  prevent_dce.inner = res;
}

void compute_main() {
  textureLoad_8fc29b();
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



