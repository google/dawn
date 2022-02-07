#version 310 es

uniform highp isampler2D arg_1_arg_2;

void textureGather_587ba3() {
  ivec4 res = textureGather(arg_1_arg_2, vec2(0.0f, 0.0f), 1);
}

vec4 vertex_main() {
  textureGather_587ba3();
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

uniform highp isampler2D arg_1_arg_2;

void textureGather_587ba3() {
  ivec4 res = textureGather(arg_1_arg_2, vec2(0.0f, 0.0f), 1);
}

void fragment_main() {
  textureGather_587ba3();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp isampler2D arg_1_arg_2;

void textureGather_587ba3() {
  ivec4 res = textureGather(arg_1_arg_2, vec2(0.0f, 0.0f), 1);
}

void compute_main() {
  textureGather_587ba3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
