#version 310 es
precision highp float;
precision highp int;


struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

void frexp_eb2421() {
  frexp_result_vec2_f32 res = frexp_result_vec2_f32(vec2(0.5f), ivec2(1));
}
void main() {
  frexp_eb2421();
}
#version 310 es


struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

void frexp_eb2421() {
  frexp_result_vec2_f32 res = frexp_result_vec2_f32(vec2(0.5f), ivec2(1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_eb2421();
}
#version 310 es


struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

struct VertexOutput {
  vec4 pos;
};

void frexp_eb2421() {
  frexp_result_vec2_f32 res = frexp_result_vec2_f32(vec2(0.5f), ivec2(1));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_eb2421();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
