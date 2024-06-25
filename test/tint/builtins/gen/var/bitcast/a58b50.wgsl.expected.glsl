#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

uint tint_bitcast_from_f16(f16vec2 src) {
  uint r = packFloat2x16(src);
  return uint(r);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint bitcast_a58b50() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  uint res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = bitcast_a58b50();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

uint tint_bitcast_from_f16(f16vec2 src) {
  uint r = packFloat2x16(src);
  return uint(r);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint bitcast_a58b50() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  uint res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void compute_main() {
  prevent_dce.inner = bitcast_a58b50();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

uint tint_bitcast_from_f16(f16vec2 src) {
  uint r = packFloat2x16(src);
  return uint(r);
}

layout(location = 0) flat out uint prevent_dce_1;
uint bitcast_a58b50() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  uint res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = bitcast_a58b50();
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
