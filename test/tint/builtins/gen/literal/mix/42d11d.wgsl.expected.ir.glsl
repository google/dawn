#version 310 es
precision highp float;
precision highp int;

void mix_42d11d() {
  vec2 res = vec2(1.0f);
}
void main() {
  mix_42d11d();
}
#version 310 es

void mix_42d11d() {
  vec2 res = vec2(1.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mix_42d11d();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void mix_42d11d() {
  vec2 res = vec2(1.0f);
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  mix_42d11d();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
