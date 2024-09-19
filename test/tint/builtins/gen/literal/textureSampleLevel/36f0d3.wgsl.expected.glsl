SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

uniform highp sampler2DArrayShadow arg_0_arg_1;

float textureSampleLevel_36f0d3() {
  float res = textureLodOffset(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1)), 0.0f), float(1u), ivec2(1));
  return res;
}

struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = textureSampleLevel_36f0d3();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'textureLodOffset for sampler2DArrayShadow' : required extension not requested: GL_EXT_texture_shadow_lod
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

uniform highp sampler2DArrayShadow arg_0_arg_1;

float textureSampleLevel_36f0d3() {
  float res = textureLodOffset(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1)), 0.0f), float(1u), ivec2(1));
  return res;
}

struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

void compute_main() {
  prevent_dce.inner = textureSampleLevel_36f0d3();
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



#version 310 es

layout(location = 0) flat out float prevent_dce_1;
uniform highp sampler2DArrayShadow arg_0_arg_1;

float textureSampleLevel_36f0d3() {
  float res = textureLodOffset(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1)), 0.0f), float(1u), ivec2(1));
  return res;
}

struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureSampleLevel_36f0d3();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  prevent_dce_1 = inner_result.prevent_dce;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'textureLodOffset for sampler2DArrayShadow' : required extension not requested: GL_EXT_texture_shadow_lod
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
