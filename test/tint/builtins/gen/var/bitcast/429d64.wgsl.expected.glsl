#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

f16vec4 tint_bitcast_to_f16(vec2 src) {
  uvec2 r = floatBitsToUint(src);
  f16vec2 v_xy = unpackFloat2x16(r.x);
  f16vec2 v_zw = unpackFloat2x16(r.y);
  return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec4 inner;
} prevent_dce;

f16vec4 bitcast_429d64() {
  vec2 arg_0 = vec2(1.0f);
  f16vec4 res = tint_bitcast_to_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  f16vec4 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = bitcast_429d64();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec4 tint_bitcast_to_f16(vec2 src) {
  uvec2 r = floatBitsToUint(src);
  f16vec2 v_xy = unpackFloat2x16(r.x);
  f16vec2 v_zw = unpackFloat2x16(r.y);
  return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec4 inner;
} prevent_dce;

f16vec4 bitcast_429d64() {
  vec2 arg_0 = vec2(1.0f);
  f16vec4 res = tint_bitcast_to_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  f16vec4 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = bitcast_429d64();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec4 tint_bitcast_to_f16(vec2 src) {
  uvec2 r = floatBitsToUint(src);
  f16vec2 v_xy = unpackFloat2x16(r.x);
  f16vec2 v_zw = unpackFloat2x16(r.y);
  return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
}

layout(location = 0) flat out f16vec4 prevent_dce_1;
f16vec4 bitcast_429d64() {
  vec2 arg_0 = vec2(1.0f);
  f16vec4 res = tint_bitcast_to_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  f16vec4 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), f16vec4(0.0hf, 0.0hf, 0.0hf, 0.0hf));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = bitcast_429d64();
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
