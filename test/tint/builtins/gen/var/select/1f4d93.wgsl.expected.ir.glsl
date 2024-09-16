#version 310 es
precision highp float;
precision highp int;

void select_1f4d93() {
  bvec2 arg_2 = bvec2(true);
  bvec2 v = arg_2;
  float v_1 = ((v.x) ? (vec2(1.0f).x) : (vec2(1.0f).x));
  vec2 res = vec2(v_1, ((v.y) ? (vec2(1.0f).y) : (vec2(1.0f).y)));
}
void main() {
  select_1f4d93();
}
#version 310 es

void select_1f4d93() {
  bvec2 arg_2 = bvec2(true);
  bvec2 v = arg_2;
  float v_1 = ((v.x) ? (vec2(1.0f).x) : (vec2(1.0f).x));
  vec2 res = vec2(v_1, ((v.y) ? (vec2(1.0f).y) : (vec2(1.0f).y)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  select_1f4d93();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void select_1f4d93() {
  bvec2 arg_2 = bvec2(true);
  bvec2 v = arg_2;
  float v_1 = ((v.x) ? (vec2(1.0f).x) : (vec2(1.0f).x));
  vec2 res = vec2(v_1, ((v.y) ? (vec2(1.0f).y) : (vec2(1.0f).y)));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  select_1f4d93();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
