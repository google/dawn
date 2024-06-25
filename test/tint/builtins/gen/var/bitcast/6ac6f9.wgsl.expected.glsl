#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

int tint_bitcast_from_f16(f16vec2 src) {
  uint r = packFloat2x16(src);
  return int(r);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

int bitcast_6ac6f9() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  int res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = bitcast_6ac6f9();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

int tint_bitcast_from_f16(f16vec2 src) {
  uint r = packFloat2x16(src);
  return int(r);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

int bitcast_6ac6f9() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  int res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

void compute_main() {
  prevent_dce.inner = bitcast_6ac6f9();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

int tint_bitcast_from_f16(f16vec2 src) {
  uint r = packFloat2x16(src);
  return int(r);
}

layout(location = 0) flat out int prevent_dce_1;
int bitcast_6ac6f9() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  int res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = bitcast_6ac6f9();
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
