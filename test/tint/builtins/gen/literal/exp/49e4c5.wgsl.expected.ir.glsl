#version 310 es
precision highp float;
precision highp int;

void exp_49e4c5() {
  float res = 2.71828174591064453125f;
}
void main() {
  exp_49e4c5();
}
#version 310 es

void exp_49e4c5() {
  float res = 2.71828174591064453125f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  exp_49e4c5();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void exp_49e4c5() {
  float res = 2.71828174591064453125f;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  exp_49e4c5();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
