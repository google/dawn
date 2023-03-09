#version 310 es

vec4 tint_select(vec4 param_0, vec4 param_1, bvec4 param_2) {
    return vec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


vec4 tint_atanh(vec4 x) {
  return tint_select(atanh(x), vec4(0.0f), greaterThanEqual(x, vec4(1.0f)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void atanh_f3e01b() {
  vec4 arg_0 = vec4(0.5f);
  vec4 res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  atanh_f3e01b();
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

vec4 tint_select(vec4 param_0, vec4 param_1, bvec4 param_2) {
    return vec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


vec4 tint_atanh(vec4 x) {
  return tint_select(atanh(x), vec4(0.0f), greaterThanEqual(x, vec4(1.0f)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void atanh_f3e01b() {
  vec4 arg_0 = vec4(0.5f);
  vec4 res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  atanh_f3e01b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

vec4 tint_select(vec4 param_0, vec4 param_1, bvec4 param_2) {
    return vec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


vec4 tint_atanh(vec4 x) {
  return tint_select(atanh(x), vec4(0.0f), greaterThanEqual(x, vec4(1.0f)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void atanh_f3e01b() {
  vec4 arg_0 = vec4(0.5f);
  vec4 res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  atanh_f3e01b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
