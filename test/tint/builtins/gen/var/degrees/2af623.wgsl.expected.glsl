#version 310 es
precision highp float;
precision highp int;

vec3 tint_degrees(vec3 param_0) {
  return param_0 * 57.29577951308232286465f;
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

vec3 degrees_2af623() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = tint_degrees(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec3 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = degrees_2af623();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

vec3 tint_degrees(vec3 param_0) {
  return param_0 * 57.29577951308232286465f;
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

vec3 degrees_2af623() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = tint_degrees(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec3 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = degrees_2af623();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

vec3 tint_degrees(vec3 param_0) {
  return param_0 * 57.29577951308232286465f;
}


layout(location = 0) flat out vec3 prevent_dce_1;
vec3 degrees_2af623() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = tint_degrees(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec3 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = degrees_2af623();
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
