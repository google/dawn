#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp samplerCubeArrayShadow arg_0_arg_1;

vec4 textureGather_7dd226() {
  vec3 arg_2 = vec3(1.0f);
  uint arg_3 = 1u;
  vec4 res = textureGather(arg_0_arg_1, vec4(arg_2, float(arg_3)), 0.0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = textureGather_7dd226();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp samplerCubeArrayShadow arg_0_arg_1;

vec4 textureGather_7dd226() {
  vec3 arg_2 = vec3(1.0f);
  uint arg_3 = 1u;
  vec4 res = textureGather(arg_0_arg_1, vec4(arg_2, float(arg_3)), 0.0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = textureGather_7dd226();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 460

layout(location = 0) flat out vec4 prevent_dce_1;
uniform highp samplerCubeArrayShadow arg_0_arg_1;

vec4 textureGather_7dd226() {
  vec3 arg_2 = vec3(1.0f);
  uint arg_3 = 1u;
  vec4 res = textureGather(arg_0_arg_1, vec4(arg_2, float(arg_3)), 0.0);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureGather_7dd226();
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
