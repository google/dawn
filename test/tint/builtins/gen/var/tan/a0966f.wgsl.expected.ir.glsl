#version 310 es
precision highp float;
precision highp int;

void tan_a0966f() {
  vec4 res = vec4(1.55740773677825927734f);
}
void main() {
  tan_a0966f();
}
#version 310 es

void tan_a0966f() {
  vec4 res = vec4(1.55740773677825927734f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tan_a0966f();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void tan_a0966f() {
  vec4 res = vec4(1.55740773677825927734f);
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tan_a0966f();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
