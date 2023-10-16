SKIP: FAILED

#version 310 es

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleLevel_cdfe0f() {
  float res = textureLodOffset(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1u)), 0.0f), float(1u), ivec2(1));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureSampleLevel_cdfe0f();
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
ERROR: 0:10: 'textureLodOffset for sampler2DArrayShadow' : required extension not requested: GL_EXT_texture_shadow_lod
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleLevel_cdfe0f() {
  float res = textureLodOffset(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1u)), 0.0f), float(1u), ivec2(1));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleLevel_cdfe0f();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'textureLodOffset for sampler2DArrayShadow' : required extension not requested: GL_EXT_texture_shadow_lod
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleLevel_cdfe0f() {
  float res = textureLodOffset(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1u)), 0.0f), float(1u), ivec2(1));
  prevent_dce.inner = res;
}

void compute_main() {
  textureSampleLevel_cdfe0f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'textureLodOffset for sampler2DArrayShadow' : required extension not requested: GL_EXT_texture_shadow_lod
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



