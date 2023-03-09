#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  mat3 inner;
} prevent_dce;

void assign_and_preserve_padding_prevent_dce(mat3 value) {
  prevent_dce.inner[0] = value[0u];
  prevent_dce.inner[1] = value[1u];
  prevent_dce.inner[2] = value[2u];
}

void transpose_854336() {
  mat3 arg_0 = mat3(vec3(1.0f), vec3(1.0f), vec3(1.0f));
  mat3 res = transpose(arg_0);
  assign_and_preserve_padding_prevent_dce(res);
}

vec4 vertex_main() {
  transpose_854336();
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
  mat3 inner;
} prevent_dce;

void assign_and_preserve_padding_prevent_dce(mat3 value) {
  prevent_dce.inner[0] = value[0u];
  prevent_dce.inner[1] = value[1u];
  prevent_dce.inner[2] = value[2u];
}

void transpose_854336() {
  mat3 arg_0 = mat3(vec3(1.0f), vec3(1.0f), vec3(1.0f));
  mat3 res = transpose(arg_0);
  assign_and_preserve_padding_prevent_dce(res);
}

void fragment_main() {
  transpose_854336();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  mat3 inner;
} prevent_dce;

void assign_and_preserve_padding_prevent_dce(mat3 value) {
  prevent_dce.inner[0] = value[0u];
  prevent_dce.inner[1] = value[1u];
  prevent_dce.inner[2] = value[2u];
}

void transpose_854336() {
  mat3 arg_0 = mat3(vec3(1.0f), vec3(1.0f), vec3(1.0f));
  mat3 res = transpose(arg_0);
  assign_and_preserve_padding_prevent_dce(res);
}

void compute_main() {
  transpose_854336();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
