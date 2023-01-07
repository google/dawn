#version 310 es

uniform highp sampler2D arg_0_1;
void textureLoad_6d376a() {
  vec4 res = texelFetch(arg_0_1, ivec2(uvec2(1u, 0u)), int(1u));
}

vec4 vertex_main() {
  textureLoad_6d376a();
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
#version 310 es
precision mediump float;

uniform highp sampler2D arg_0_1;
void textureLoad_6d376a() {
  vec4 res = texelFetch(arg_0_1, ivec2(uvec2(1u, 0u)), int(1u));
}

void fragment_main() {
  textureLoad_6d376a();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp sampler2D arg_0_1;
void textureLoad_6d376a() {
  vec4 res = texelFetch(arg_0_1, ivec2(uvec2(1u, 0u)), int(1u));
}

void compute_main() {
  textureLoad_6d376a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
