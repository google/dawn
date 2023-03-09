#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

void select_51b047() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  bool arg_2 = true;
  uvec2 res = (arg_2 ? arg_1 : arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  select_51b047();
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
  uvec2 inner;
} prevent_dce;

void select_51b047() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  bool arg_2 = true;
  uvec2 res = (arg_2 ? arg_1 : arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  select_51b047();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec2 inner;
} prevent_dce;

void select_51b047() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  bool arg_2 = true;
  uvec2 res = (arg_2 ? arg_1 : arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  select_51b047();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
