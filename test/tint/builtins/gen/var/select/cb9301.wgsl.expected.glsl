#version 310 es

bvec2 tint_select(bvec2 param_0, bvec2 param_1, bvec2 param_2) {
    return bvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void select_cb9301() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bvec2 arg_2 = bvec2(true);
  bvec2 res = tint_select(arg_0, arg_1, arg_2);
  prevent_dce.inner = (all(equal(res, bvec2(false))) ? 1 : 0);
}

vec4 vertex_main() {
  select_cb9301();
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

bvec2 tint_select(bvec2 param_0, bvec2 param_1, bvec2 param_2) {
    return bvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void select_cb9301() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bvec2 arg_2 = bvec2(true);
  bvec2 res = tint_select(arg_0, arg_1, arg_2);
  prevent_dce.inner = (all(equal(res, bvec2(false))) ? 1 : 0);
}

void fragment_main() {
  select_cb9301();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

bvec2 tint_select(bvec2 param_0, bvec2 param_1, bvec2 param_2) {
    return bvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void select_cb9301() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bvec2 arg_2 = bvec2(true);
  bvec2 res = tint_select(arg_0, arg_1, arg_2);
  prevent_dce.inner = (all(equal(res, bvec2(false))) ? 1 : 0);
}

void compute_main() {
  select_cb9301();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
