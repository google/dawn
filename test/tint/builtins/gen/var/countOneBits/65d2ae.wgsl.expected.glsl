#version 310 es

void countOneBits_65d2ae() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = ivec3(bitCount(arg_0));
}

vec4 vertex_main() {
  countOneBits_65d2ae();
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

void countOneBits_65d2ae() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = ivec3(bitCount(arg_0));
}

void fragment_main() {
  countOneBits_65d2ae();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void countOneBits_65d2ae() {
  ivec3 arg_0 = ivec3(1);
  ivec3 res = ivec3(bitCount(arg_0));
}

void compute_main() {
  countOneBits_65d2ae();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
