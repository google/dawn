#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

vec2 tint_bitcast_from_f16(f16vec4 src) {
  uvec2 r = uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));
  return uintBitsToFloat(r);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

vec2 bitcast_2a6e58() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  vec2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec2 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = bitcast_2a6e58();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

vec2 tint_bitcast_from_f16(f16vec4 src) {
  uvec2 r = uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));
  return uintBitsToFloat(r);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

vec2 bitcast_2a6e58() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  vec2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec2 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = bitcast_2a6e58();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

vec2 tint_bitcast_from_f16(f16vec4 src) {
  uvec2 r = uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));
  return uintBitsToFloat(r);
}

layout(location = 0) flat out vec2 prevent_dce_1;
vec2 bitcast_2a6e58() {
  f16vec4 arg_0 = f16vec4(1.0hf);
  vec2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec2 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = bitcast_2a6e58();
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
