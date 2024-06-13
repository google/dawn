#version 310 es
precision highp float;
precision highp int;

struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};

modf_result_vec4_f32 tint_modf(vec4 param_0) {
  modf_result_vec4_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void modf_4bfced() {
  vec4 arg_0 = vec4(-1.5f);
  modf_result_vec4_f32 res = tint_modf(arg_0);
}

struct VertexOutput {
  vec4 pos;
};

void fragment_main() {
  modf_4bfced();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};

modf_result_vec4_f32 tint_modf(vec4 param_0) {
  modf_result_vec4_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void modf_4bfced() {
  vec4 arg_0 = vec4(-1.5f);
  modf_result_vec4_f32 res = tint_modf(arg_0);
}

struct VertexOutput {
  vec4 pos;
};

void compute_main() {
  modf_4bfced();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};

modf_result_vec4_f32 tint_modf(vec4 param_0) {
  modf_result_vec4_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void modf_4bfced() {
  vec4 arg_0 = vec4(-1.5f);
  modf_result_vec4_f32 res = tint_modf(arg_0);
}

struct VertexOutput {
  vec4 pos;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_4bfced();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
