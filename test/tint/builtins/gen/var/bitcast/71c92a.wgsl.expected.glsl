#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 r = uvec2(src);
  f16vec2 v_xy = unpackFloat2x16(r.x);
  f16vec2 v_zw = unpackFloat2x16(r.y);
  return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec4 inner;
} prevent_dce;

void bitcast_71c92a() {
  ivec2 arg_0 = ivec2(1);
  f16vec4 res = tint_bitcast_to_f16(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  bitcast_71c92a();
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
precision highp int;

f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 r = uvec2(src);
  f16vec2 v_xy = unpackFloat2x16(r.x);
  f16vec2 v_zw = unpackFloat2x16(r.y);
  return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec4 inner;
} prevent_dce;

void bitcast_71c92a() {
  ivec2 arg_0 = ivec2(1);
  f16vec4 res = tint_bitcast_to_f16(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  bitcast_71c92a();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 r = uvec2(src);
  f16vec2 v_xy = unpackFloat2x16(r.x);
  f16vec2 v_zw = unpackFloat2x16(r.y);
  return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec4 inner;
} prevent_dce;

void bitcast_71c92a() {
  ivec2 arg_0 = ivec2(1);
  f16vec4 res = tint_bitcast_to_f16(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  bitcast_71c92a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
