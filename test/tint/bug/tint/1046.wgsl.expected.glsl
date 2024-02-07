#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in vec4 view_position_1;
layout(location = 1) in vec4 normal_1;
layout(location = 2) in vec2 uv_1;
layout(location = 3) in vec4 color_1;
layout(location = 0) out vec4 color_2;
struct PointLight {
  vec4 position;
};

struct Uniforms {
  mat4 worldView;
  mat4 proj;
  uint numPointLights;
  uint color_source;
  uint pad;
  uint pad_1;
  vec4 color;
};

layout(binding = 0, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

layout(binding = 1, std430) buffer PointLights_ssbo {
  PointLight values[];
} pointLights;

struct FragmentInput {
  vec4 position;
  vec4 view_position;
  vec4 normal;
  vec2 uv;
  vec4 color;
};

struct FragmentOutput {
  vec4 color;
};

FragmentOutput tint_symbol(FragmentInput fragment) {
  FragmentOutput tint_symbol_1 = FragmentOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_1.color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  return tint_symbol_1;
}

void main() {
  FragmentInput tint_symbol_2 = FragmentInput(gl_FragCoord, view_position_1, normal_1, uv_1, color_1);
  FragmentOutput inner_result = tint_symbol(tint_symbol_2);
  color_2 = inner_result.color;
  return;
}
