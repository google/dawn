#version 310 es
precision highp float;
precision highp int;

void normalize_584e47() {
  vec2 res = vec2(0.70710676908493041992f);
}
void main() {
  normalize_584e47();
}
#version 310 es

void normalize_584e47() {
  vec2 res = vec2(0.70710676908493041992f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  normalize_584e47();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void normalize_584e47() {
  vec2 res = vec2(0.70710676908493041992f);
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  normalize_584e47();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
