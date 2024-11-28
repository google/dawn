#version 310 es
precision highp float;
precision highp int;


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp sampler2D arg_0;
vec4 textureLoad_1373dc() {
  uint arg_1 = 1u;
  int arg_2 = 1;
  uint v_2 = arg_1;
  uint v_3 = (v_1.inner.tint_builtin_value_0 - 1u);
  uint v_4 = min(uint(arg_2), v_3);
  ivec2 v_5 = ivec2(uvec2(min(v_2, (uvec2(textureSize(arg_0, int(v_4))).x - 1u)), 0u));
  vec4 res = texelFetch(arg_0, v_5, int(v_4));
  return res;
}
void main() {
  v.inner = textureLoad_1373dc();
}
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp sampler2D arg_0;
vec4 textureLoad_1373dc() {
  uint arg_1 = 1u;
  int arg_2 = 1;
  uint v_2 = arg_1;
  uint v_3 = (v_1.inner.tint_builtin_value_0 - 1u);
  uint v_4 = min(uint(arg_2), v_3);
  ivec2 v_5 = ivec2(uvec2(min(v_2, (uvec2(textureSize(arg_0, int(v_4))).x - 1u)), 0u));
  vec4 res = texelFetch(arg_0, v_5, int(v_4));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_1373dc();
}
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v;
uniform highp sampler2D arg_0;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 textureLoad_1373dc() {
  uint arg_1 = 1u;
  int arg_2 = 1;
  uint v_1 = arg_1;
  uint v_2 = (v.inner.tint_builtin_value_0 - 1u);
  uint v_3 = min(uint(arg_2), v_2);
  ivec2 v_4 = ivec2(uvec2(min(v_1, (uvec2(textureSize(arg_0, int(v_3))).x - 1u)), 0u));
  vec4 res = texelFetch(arg_0, v_4, int(v_3));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_1373dc();
  return tint_symbol;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = v_5.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
