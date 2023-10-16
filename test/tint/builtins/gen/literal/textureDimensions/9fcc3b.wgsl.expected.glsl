SKIP: FAILED

#version 310 es

uniform highp samplerCubeArray arg_0_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

void textureDimensions_9fcc3b() {
  uvec2 res = uvec2(textureSize(arg_0_1, 0).xy);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureDimensions_9fcc3b();
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
ERROR: 0:3: 'samplerCubeArray' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;

uniform highp samplerCubeArray arg_0_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

void textureDimensions_9fcc3b() {
  uvec2 res = uvec2(textureSize(arg_0_1, 0).xy);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureDimensions_9fcc3b();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform highp samplerCubeArray arg_0_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

void textureDimensions_9fcc3b() {
  uvec2 res = uvec2(textureSize(arg_0_1, 0).xy);
  prevent_dce.inner = res;
}

void compute_main() {
  textureDimensions_9fcc3b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'samplerCubeArray' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



