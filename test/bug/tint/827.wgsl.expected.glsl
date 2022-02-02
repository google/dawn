SKIP: FAILED

#version 310 es
precision mediump float;

const uint width = 128u;
layout(binding = 1) buffer Result_1 {
  float values[];
} result;
uniform highp sampler2DShadow tex_1;
void tint_symbol(uvec3 GlobalInvocationId) {
  result.values[((GlobalInvocationId.y * width) + GlobalInvocationId.x)] = texelFetch(tex_1, ivec2(int(GlobalInvocationId.x), int(GlobalInvocationId.y)), 0).x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_GlobalInvocationID);
  return;
}
Error parsing GLSL shader:
ERROR: 0:10: 'texelFetch' : no matching overloaded function found 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



