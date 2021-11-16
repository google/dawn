SKIP: FAILED

#version 310 es
precision mediump float;

struct PointLight {
  vec4 position;
};

layout (binding = 0) uniform Uniforms_1 {
  mat4 worldView;
  mat4 proj;
  uint numPointLights;
  uint color_source;
  vec4 color;
} uniforms;
layout (binding = 1) buffer PointLights_1 {
  PointLight values[];
} pointLights;

uniform highp sampler2D myTexture;

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
struct tint_symbol_3 {
  vec4 view_position;
  vec4 normal;
  vec2 uv;
  vec4 color;
  vec4 position;
};
struct tint_symbol_4 {
  vec4 color;
};

FragmentOutput tint_symbol_inner(FragmentInput fragment) {
  FragmentOutput tint_symbol_1 = FragmentOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_1.color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  return tint_symbol_1;
}

tint_symbol_4 tint_symbol(tint_symbol_3 tint_symbol_2) {
  FragmentInput tint_symbol_5 = FragmentInput(tint_symbol_2.position, tint_symbol_2.view_position, tint_symbol_2.normal, tint_symbol_2.uv, tint_symbol_2.color);
  FragmentOutput inner_result = tint_symbol_inner(tint_symbol_5);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.color = inner_result.color;
  return wrapper_result;
}
in vec4 view_position;
in vec4 normal;
in vec2 uv;
in vec4 color;
out vec4 color;
void main() {
  tint_symbol_3 inputs;
  inputs.view_position = view_position;
  inputs.normal = normal;
  inputs.uv = uv;
  inputs.color = color;
  inputs.position = gl_FragCoord;
  tint_symbol_4 outputs;
  outputs = tint_symbol(inputs);
  color = outputs.color;
}


Error parsing GLSL shader:
ERROR: 0:59: 'color' : redefinition 
ERROR: 1 compilation errors.  No code generated.



