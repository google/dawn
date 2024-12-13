//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

void tanh_6289fd() {
  vec3 res = vec3(0.76159417629241943359f);
}
void main() {
  tanh_6289fd();
}
//
// compute_main
//
#version 310 es

void tanh_6289fd() {
  vec3 res = vec3(0.76159417629241943359f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tanh_6289fd();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void tanh_6289fd() {
  vec3 res = vec3(0.76159417629241943359f);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  tanh_6289fd();
  return v;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
