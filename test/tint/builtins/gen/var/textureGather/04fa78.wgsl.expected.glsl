SKIP: FAILED

#version 310 es

uniform highp isamplerCubeArray arg_1_arg_2;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureGather_04fa78() {
  vec3 arg_3 = vec3(1.0f);
  uint arg_4 = 1u;
  ivec4 res = textureGather(arg_1_arg_2, vec4(arg_3, float(arg_4)), int(1u));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureGather_04fa78();
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
ERROR: 0:3: 'isamplerCubeArray' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;

uniform highp isamplerCubeArray arg_1_arg_2;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureGather_04fa78() {
  vec3 arg_3 = vec3(1.0f);
  uint arg_4 = 1u;
  ivec4 res = textureGather(arg_1_arg_2, vec4(arg_3, float(arg_4)), int(1u));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureGather_04fa78();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'isamplerCubeArray' : Reserved word. 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform highp isamplerCubeArray arg_1_arg_2;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void textureGather_04fa78() {
  vec3 arg_3 = vec3(1.0f);
  uint arg_4 = 1u;
  ivec4 res = textureGather(arg_1_arg_2, vec4(arg_3, float(arg_4)), int(1u));
  prevent_dce.inner = res;
}

void compute_main() {
  textureGather_04fa78();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'isamplerCubeArray' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



