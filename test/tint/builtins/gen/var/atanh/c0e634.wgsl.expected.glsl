#version 310 es

vec2 tint_select(vec2 param_0, vec2 param_1, bvec2 param_2) {
    return vec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


vec2 tint_atanh(vec2 x) {
  return tint_select(atanh(x), vec2(0.0f), greaterThanEqual(x, vec2(1.0f)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

void atanh_c0e634() {
  vec2 arg_0 = vec2(0.5f);
  vec2 res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  atanh_c0e634();
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

vec2 tint_select(vec2 param_0, vec2 param_1, bvec2 param_2) {
    return vec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


vec2 tint_atanh(vec2 x) {
  return tint_select(atanh(x), vec2(0.0f), greaterThanEqual(x, vec2(1.0f)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

void atanh_c0e634() {
  vec2 arg_0 = vec2(0.5f);
  vec2 res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  atanh_c0e634();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

vec2 tint_select(vec2 param_0, vec2 param_1, bvec2 param_2) {
    return vec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


vec2 tint_atanh(vec2 x) {
  return tint_select(atanh(x), vec2(0.0f), greaterThanEqual(x, vec2(1.0f)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

void atanh_c0e634() {
  vec2 arg_0 = vec2(0.5f);
  vec2 res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  atanh_c0e634();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
