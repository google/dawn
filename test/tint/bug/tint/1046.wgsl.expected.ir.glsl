#version 310 es
precision highp float;
precision highp int;


struct Uniforms {
  mat4 worldView;
  mat4 proj;
  uint numPointLights;
  uint color_source;
  vec4 color;
};

struct PointLight {
  vec4 position;
};

struct FragmentOutput {
  vec4 color;
};

struct FragmentInput {
  vec4 position;
  vec4 view_position;
  vec4 normal;
  vec2 uv;
  vec4 color;
};

layout(binding = 0, std140)
uniform tint_symbol_3_1_ubo {
  Uniforms tint_symbol_2;
} v;
layout(binding = 1, std430)
buffer PointLights_1_ssbo {
  PointLight values[];
} pointLights;
uniform highp sampler2D myTexture;
layout(location = 0) in vec4 tint_symbol_loc0_Input;
layout(location = 1) in vec4 tint_symbol_loc1_Input;
layout(location = 2) in vec2 tint_symbol_loc2_Input;
layout(location = 3) in vec4 tint_symbol_loc3_Input;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
FragmentOutput tint_symbol_inner(FragmentInput fragment) {
  FragmentOutput tint_symbol_1 = FragmentOutput(vec4(0.0f));
  tint_symbol_1.color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  return tint_symbol_1;
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner(FragmentInput(gl_FragCoord, tint_symbol_loc0_Input, tint_symbol_loc1_Input, tint_symbol_loc2_Input, tint_symbol_loc3_Input)).color;
}
