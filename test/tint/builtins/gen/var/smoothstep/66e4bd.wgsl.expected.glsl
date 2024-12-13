//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

void smoothstep_66e4bd() {
  vec3 res = vec3(0.5f);
}
void main() {
  smoothstep_66e4bd();
}
//
// compute_main
//
#version 310 es

void smoothstep_66e4bd() {
  vec3 res = vec3(0.5f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  smoothstep_66e4bd();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void smoothstep_66e4bd() {
  vec3 res = vec3(0.5f);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  smoothstep_66e4bd();
  return v;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
