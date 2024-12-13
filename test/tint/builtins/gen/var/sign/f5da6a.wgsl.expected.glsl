//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

void sign_f5da6a() {
  vec4 res = vec4(1.0f);
}
void main() {
  sign_f5da6a();
}
//
// compute_main
//
#version 310 es

void sign_f5da6a() {
  vec4 res = vec4(1.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  sign_f5da6a();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void sign_f5da6a() {
  vec4 res = vec4(1.0f);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  sign_f5da6a();
  return v;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
