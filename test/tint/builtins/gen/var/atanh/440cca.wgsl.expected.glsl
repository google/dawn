#version 310 es
precision highp float;
precision highp int;

vec3 tint_atanh(vec3 x) {
  return mix(atanh(x), vec3(0.0f), greaterThanEqual(x, vec3(1.0f)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

vec3 atanh_440cca() {
  vec3 arg_0 = vec3(0.5f);
  vec3 res = tint_atanh(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec3 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = atanh_440cca();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

vec3 tint_atanh(vec3 x) {
  return mix(atanh(x), vec3(0.0f), greaterThanEqual(x, vec3(1.0f)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

vec3 atanh_440cca() {
  vec3 arg_0 = vec3(0.5f);
  vec3 res = tint_atanh(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec3 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = atanh_440cca();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

vec3 tint_atanh(vec3 x) {
  return mix(atanh(x), vec3(0.0f), greaterThanEqual(x, vec3(1.0f)));
}

layout(location = 0) flat out vec3 prevent_dce_1;
vec3 atanh_440cca() {
  vec3 arg_0 = vec3(0.5f);
  vec3 res = tint_atanh(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec3 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = atanh_440cca();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  prevent_dce_1 = inner_result.prevent_dce;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
