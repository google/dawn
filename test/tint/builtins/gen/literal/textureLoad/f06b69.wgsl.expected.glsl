SKIP: FAILED

#version 310 es

layout(r32i) uniform highp iimage2D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureLoad_f06b69() {
  ivec4 res = texelFetch(arg_0, ivec2(1, 0));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureLoad_f06b69();
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
Error parsing GLSL shader:
ERROR: 0:9: 'texelFetch' : no matching overloaded function found 
ERROR: 0:9: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of int'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;

layout(r32i) uniform highp iimage2D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureLoad_f06b69() {
  ivec4 res = texelFetch(arg_0, ivec2(1, 0));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureLoad_f06b69();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:10: 'texelFetch' : no matching overloaded function found 
ERROR: 0:10: '=' :  cannot convert from ' const float' to ' temp mediump 4-component vector of int'
ERROR: 0:10: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

layout(r32i) uniform highp iimage2D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureLoad_f06b69() {
  ivec4 res = texelFetch(arg_0, ivec2(1, 0));
  prevent_dce.inner = res;
}

void compute_main() {
  textureLoad_f06b69();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:9: 'texelFetch' : no matching overloaded function found 
ERROR: 0:9: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of int'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



