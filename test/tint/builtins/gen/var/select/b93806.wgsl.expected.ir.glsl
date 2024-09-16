#version 310 es
precision highp float;
precision highp int;

void select_b93806() {
  bvec3 arg_2 = bvec3(true);
  bvec3 v = arg_2;
  int v_1 = ((v.x) ? (ivec3(1).x) : (ivec3(1).x));
  int v_2 = ((v.y) ? (ivec3(1).y) : (ivec3(1).y));
  ivec3 res = ivec3(v_1, v_2, ((v.z) ? (ivec3(1).z) : (ivec3(1).z)));
}
void main() {
  select_b93806();
}
#version 310 es

void select_b93806() {
  bvec3 arg_2 = bvec3(true);
  bvec3 v = arg_2;
  int v_1 = ((v.x) ? (ivec3(1).x) : (ivec3(1).x));
  int v_2 = ((v.y) ? (ivec3(1).y) : (ivec3(1).y));
  ivec3 res = ivec3(v_1, v_2, ((v.z) ? (ivec3(1).z) : (ivec3(1).z)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  select_b93806();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void select_b93806() {
  bvec3 arg_2 = bvec3(true);
  bvec3 v = arg_2;
  int v_1 = ((v.x) ? (ivec3(1).x) : (ivec3(1).x));
  int v_2 = ((v.y) ? (ivec3(1).y) : (ivec3(1).y));
  ivec3 res = ivec3(v_1, v_2, ((v.z) ? (ivec3(1).z) : (ivec3(1).z)));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  select_b93806();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
