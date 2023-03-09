#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void sign_b8f634() {
  vec4 res = vec4(1.0f);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  sign_b8f634();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void sign_b8f634() {
  vec4 res = vec4(1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  sign_b8f634();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void sign_b8f634() {
  vec4 res = vec4(1.0f);
  prevent_dce.inner = res;
}

void compute_main() {
  sign_b8f634();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
