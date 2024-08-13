#version 310 es
precision highp float;
precision highp int;

struct tint_symbol_1 {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_2_block_ubo {
  tint_symbol_1 inner;
} tint_symbol_2;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint textureNumSamples_c1a777() {
  uint res = tint_symbol_2.inner.texture_builtin_value_0;
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = textureNumSamples_c1a777();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct tint_symbol_1 {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_2_block_ubo {
  tint_symbol_1 inner;
} tint_symbol_2;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint textureNumSamples_c1a777() {
  uint res = tint_symbol_2.inner.texture_builtin_value_0;
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

void compute_main() {
  prevent_dce.inner = textureNumSamples_c1a777();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

layout(location = 0) flat out uint prevent_dce_1;
struct tint_symbol_1 {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_2_block_ubo {
  tint_symbol_1 inner;
} tint_symbol_2;

uint textureNumSamples_c1a777() {
  uint res = tint_symbol_2.inner.texture_builtin_value_0;
  return res;
}

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureNumSamples_c1a777();
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
