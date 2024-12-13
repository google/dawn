//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

void abs_577d6e() {
  ivec2 res = ivec2(1);
}
void main() {
  abs_577d6e();
}
//
// compute_main
//
#version 310 es

void abs_577d6e() {
  ivec2 res = ivec2(1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  abs_577d6e();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void abs_577d6e() {
  ivec2 res = ivec2(1);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  abs_577d6e();
  return v;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
