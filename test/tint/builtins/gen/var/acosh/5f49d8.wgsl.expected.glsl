#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec2 tint_select(f16vec2 param_0, f16vec2 param_1, bvec2 param_2) {
    return f16vec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


f16vec2 tint_acosh(f16vec2 x) {
  return tint_select(acosh(x), f16vec2(0.0hf), lessThan(x, f16vec2(1.0hf)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec2 inner;
} prevent_dce;

void acosh_5f49d8() {
  f16vec2 arg_0 = f16vec2(1.54296875hf);
  f16vec2 res = tint_acosh(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  acosh_5f49d8();
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
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;

f16vec2 tint_select(f16vec2 param_0, f16vec2 param_1, bvec2 param_2) {
    return f16vec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


f16vec2 tint_acosh(f16vec2 x) {
  return tint_select(acosh(x), f16vec2(0.0hf), lessThan(x, f16vec2(1.0hf)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec2 inner;
} prevent_dce;

void acosh_5f49d8() {
  f16vec2 arg_0 = f16vec2(1.54296875hf);
  f16vec2 res = tint_acosh(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  acosh_5f49d8();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec2 tint_select(f16vec2 param_0, f16vec2 param_1, bvec2 param_2) {
    return f16vec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


f16vec2 tint_acosh(f16vec2 x) {
  return tint_select(acosh(x), f16vec2(0.0hf), lessThan(x, f16vec2(1.0hf)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec2 inner;
} prevent_dce;

void acosh_5f49d8() {
  f16vec2 arg_0 = f16vec2(1.54296875hf);
  f16vec2 res = tint_acosh(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  acosh_5f49d8();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
