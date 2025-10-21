#version 310 es
precision highp float;
precision highp int;


struct PointLight {
  vec4 position;
};

struct Uniforms {
  mat4 worldView;
  mat4 proj;
  uint numPointLights;
  uint color_source;
  vec4 color;
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
uniform f_uniforms_block_ubo {
  uvec4 inner[10];
} v;
layout(binding = 1, std430)
buffer f_PointLights_ssbo {
  PointLight values[];
} pointLights;
uniform highp sampler2D f_myTexture;
layout(location = 0) in vec4 tint_interstage_location0;
layout(location = 1) in vec4 tint_interstage_location1;
layout(location = 2) in vec2 tint_interstage_location2;
layout(location = 3) in vec4 tint_interstage_location3;
layout(location = 0) out vec4 main_loc0_Output;
mat4 v_1(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
Uniforms v_2(uint start_byte_offset) {
  mat4 v_3 = v_1(start_byte_offset);
  mat4 v_4 = v_1((64u + start_byte_offset));
  uvec4 v_5 = v.inner[((128u + start_byte_offset) / 16u)];
  uvec4 v_6 = v.inner[((132u + start_byte_offset) / 16u)];
  return Uniforms(v_3, v_4, v_5[(((128u + start_byte_offset) % 16u) / 4u)], v_6[(((132u + start_byte_offset) % 16u) / 4u)], uintBitsToFloat(v.inner[((144u + start_byte_offset) / 16u)]));
}
FragmentOutput main_inner(FragmentInput fragment) {
  FragmentOutput v_7 = FragmentOutput(vec4(0.0f));
  v_7.color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  v_2(0u);
  return v_7;
}
void main() {
  main_loc0_Output = main_inner(FragmentInput(gl_FragCoord, tint_interstage_location0, tint_interstage_location1, tint_interstage_location2, tint_interstage_location3)).color;
}
