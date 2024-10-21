#version 310 es
precision highp float;
precision highp int;


struct modf_result_vec2_f32 {
  vec2 fract;
  vec2 whole;
};

void modf_732aa6() {
  modf_result_vec2_f32 res = modf_result_vec2_f32(vec2(-0.5f), vec2(-1.0f));
}
void main() {
  modf_732aa6();
}
#version 310 es


struct modf_result_vec2_f32 {
  vec2 fract;
  vec2 whole;
};

void modf_732aa6() {
  modf_result_vec2_f32 res = modf_result_vec2_f32(vec2(-0.5f), vec2(-1.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_732aa6();
}
#version 310 es


struct modf_result_vec2_f32 {
  vec2 fract;
  vec2 whole;
};

struct VertexOutput {
  vec4 pos;
};

void modf_732aa6() {
  modf_result_vec2_f32 res = modf_result_vec2_f32(vec2(-0.5f), vec2(-1.0f));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_732aa6();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
