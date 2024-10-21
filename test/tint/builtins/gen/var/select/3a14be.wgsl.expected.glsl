#version 310 es
precision highp float;
precision highp int;

void select_3a14be() {
  bool arg_2 = true;
  ivec2 res = mix(ivec2(1), ivec2(1), bvec2(arg_2));
}
void main() {
  select_3a14be();
}
#version 310 es

void select_3a14be() {
  bool arg_2 = true;
  ivec2 res = mix(ivec2(1), ivec2(1), bvec2(arg_2));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  select_3a14be();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void select_3a14be() {
  bool arg_2 = true;
  ivec2 res = mix(ivec2(1), ivec2(1), bvec2(arg_2));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  select_3a14be();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
