#version 310 es

uniform highp sampler3D arg_0_arg_1;

void textureSampleLevel_9bd37b() {
  vec4 res = textureLodOffset(arg_0_arg_1, vec3(0.0f, 0.0f, 0.0f), 1.0f, ivec3(0, 0, 0));
}

vec4 vertex_main() {
  textureSampleLevel_9bd37b();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

uniform highp sampler3D arg_0_arg_1;

void textureSampleLevel_9bd37b() {
  vec4 res = textureLodOffset(arg_0_arg_1, vec3(0.0f, 0.0f, 0.0f), 1.0f, ivec3(0, 0, 0));
}

void fragment_main() {
  textureSampleLevel_9bd37b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp sampler3D arg_0_arg_1;

void textureSampleLevel_9bd37b() {
  vec4 res = textureLodOffset(arg_0_arg_1, vec3(0.0f, 0.0f, 0.0f), 1.0f, ivec3(0, 0, 0));
}

void compute_main() {
  textureSampleLevel_9bd37b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
