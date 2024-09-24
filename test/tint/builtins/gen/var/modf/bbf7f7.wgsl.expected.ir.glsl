#version 310 es
precision highp float;
precision highp int;


struct modf_result_f32 {
  float fract;
  float whole;
};

void modf_bbf7f7() {
  float arg_0 = -1.5f;
  modf_result_f32 v = modf_result_f32(0.0f, 0.0f);
  v.fract = modf(arg_0, v.whole);
  modf_result_f32 res = v;
}
void main() {
  modf_bbf7f7();
}
#version 310 es


struct modf_result_f32 {
  float fract;
  float whole;
};

void modf_bbf7f7() {
  float arg_0 = -1.5f;
  modf_result_f32 v = modf_result_f32(0.0f, 0.0f);
  v.fract = modf(arg_0, v.whole);
  modf_result_f32 res = v;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_bbf7f7();
}
#version 310 es


struct modf_result_f32 {
  float fract;
  float whole;
};

struct VertexOutput {
  vec4 pos;
};

void modf_bbf7f7() {
  float arg_0 = -1.5f;
  modf_result_f32 v = modf_result_f32(0.0f, 0.0f);
  v.fract = modf(arg_0, v.whole);
  modf_result_f32 res = v;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_bbf7f7();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
