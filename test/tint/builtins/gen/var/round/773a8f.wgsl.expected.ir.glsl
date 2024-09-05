#version 310 es
precision highp float;
precision highp int;

void round_773a8f() {
  float res = 4.0f;
}
void main() {
  round_773a8f();
}
#version 310 es

void round_773a8f() {
  float res = 4.0f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  round_773a8f();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void round_773a8f() {
  float res = 4.0f;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  round_773a8f();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
