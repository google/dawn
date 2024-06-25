#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

uniform highp isampler2DArray arg_1_arg_2;

ivec4 textureGather_9ab41e() {
  ivec4 res = textureGatherOffset(arg_1_arg_2, vec3(vec2(1.0f), float(1)), ivec2(1), int(1u));
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = textureGather_9ab41e();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

uniform highp isampler2DArray arg_1_arg_2;

ivec4 textureGather_9ab41e() {
  ivec4 res = textureGatherOffset(arg_1_arg_2, vec3(vec2(1.0f), float(1)), ivec2(1), int(1u));
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = textureGather_9ab41e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

layout(location = 0) flat out ivec4 prevent_dce_1;
uniform highp isampler2DArray arg_1_arg_2;

ivec4 textureGather_9ab41e() {
  ivec4 res = textureGatherOffset(arg_1_arg_2, vec3(vec2(1.0f), float(1)), ivec2(1), int(1u));
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), ivec4(0, 0, 0, 0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureGather_9ab41e();
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
