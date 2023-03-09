#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  mat2x3 inner;
} prevent_dce;

void assign_and_preserve_padding_prevent_dce(mat2x3 value) {
  prevent_dce.inner[0] = value[0u];
  prevent_dce.inner[1] = value[1u];
}

void transpose_ed4bdc() {
  mat2x3 res = mat2x3(vec3(1.0f), vec3(1.0f));
  assign_and_preserve_padding_prevent_dce(res);
}

vec4 vertex_main() {
  transpose_ed4bdc();
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
  mat2x3 inner;
} prevent_dce;

void assign_and_preserve_padding_prevent_dce(mat2x3 value) {
  prevent_dce.inner[0] = value[0u];
  prevent_dce.inner[1] = value[1u];
}

void transpose_ed4bdc() {
  mat2x3 res = mat2x3(vec3(1.0f), vec3(1.0f));
  assign_and_preserve_padding_prevent_dce(res);
}

void fragment_main() {
  transpose_ed4bdc();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  mat2x3 inner;
} prevent_dce;

void assign_and_preserve_padding_prevent_dce(mat2x3 value) {
  prevent_dce.inner[0] = value[0u];
  prevent_dce.inner[1] = value[1u];
}

void transpose_ed4bdc() {
  mat2x3 res = mat2x3(vec3(1.0f), vec3(1.0f));
  assign_and_preserve_padding_prevent_dce(res);
}

void compute_main() {
  transpose_ed4bdc();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
