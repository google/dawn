//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

void clamp_d396af() {
  ivec4 res = ivec4(1);
}
void main() {
  clamp_d396af();
}
//
// compute_main
//
#version 310 es

void clamp_d396af() {
  ivec4 res = ivec4(1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  clamp_d396af();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void clamp_d396af() {
  ivec4 res = ivec4(1);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  clamp_d396af();
  return v;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
