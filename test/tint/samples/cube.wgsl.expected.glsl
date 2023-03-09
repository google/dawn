#version 310 es

layout(location = 0) in vec4 cur_position_1;
layout(location = 1) in vec4 color_1;
layout(location = 0) out vec4 vtxFragColor_1;
struct Uniforms {
  mat4 modelViewProjectionMatrix;
};

layout(binding = 0, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

struct VertexInput {
  vec4 cur_position;
  vec4 color;
};

struct VertexOutput {
  vec4 vtxFragColor;
  vec4 Position;
};

VertexOutput vtx_main(VertexInput tint_symbol) {
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_1.Position = (uniforms.inner.modelViewProjectionMatrix * tint_symbol.cur_position);
  tint_symbol_1.vtxFragColor = tint_symbol.color;
  return tint_symbol_1;
}

void main() {
  gl_PointSize = 1.0;
  VertexInput tint_symbol_2 = VertexInput(cur_position_1, color_1);
  VertexOutput inner_result = vtx_main(tint_symbol_2);
  vtxFragColor_1 = inner_result.vtxFragColor;
  gl_Position = inner_result.Position;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;

layout(location = 0) in vec4 fragColor_1;
layout(location = 0) out vec4 value;
struct Uniforms {
  mat4 modelViewProjectionMatrix;
};

struct VertexInput {
  vec4 cur_position;
  vec4 color;
};

struct VertexOutput {
  vec4 vtxFragColor;
  vec4 Position;
};

vec4 frag_main(vec4 fragColor) {
  return fragColor;
}

void main() {
  vec4 inner_result = frag_main(fragColor_1);
  value = inner_result;
  return;
}
