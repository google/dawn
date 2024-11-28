#version 310 es
precision highp float;
precision highp int;


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp usampler2DArray arg_0;
uvec4 textureLoad_7ab4df() {
  uint v_2 = min(1u, (uint(textureSize(arg_0, 0).z) - 1u));
  uint v_3 = (v_1.inner.tint_builtin_value_0 - 1u);
  uint v_4 = min(uint(1), v_3);
  uvec2 v_5 = (uvec2(textureSize(arg_0, int(v_4)).xy) - uvec2(1u));
  ivec2 v_6 = ivec2(min(uvec2(ivec2(1)), v_5));
  ivec3 v_7 = ivec3(v_6, int(v_2));
  uvec4 res = texelFetch(arg_0, v_7, int(v_4));
  return res;
}
void main() {
  v.inner = textureLoad_7ab4df();
}
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp usampler2DArray arg_0;
uvec4 textureLoad_7ab4df() {
  uint v_2 = min(1u, (uint(textureSize(arg_0, 0).z) - 1u));
  uint v_3 = (v_1.inner.tint_builtin_value_0 - 1u);
  uint v_4 = min(uint(1), v_3);
  uvec2 v_5 = (uvec2(textureSize(arg_0, int(v_4)).xy) - uvec2(1u));
  ivec2 v_6 = ivec2(min(uvec2(ivec2(1)), v_5));
  ivec3 v_7 = ivec3(v_6, int(v_2));
  uvec4 res = texelFetch(arg_0, v_7, int(v_4));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_7ab4df();
}
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v;
uniform highp usampler2DArray arg_0;
layout(location = 0) flat out uvec4 vertex_main_loc0_Output;
uvec4 textureLoad_7ab4df() {
  uint v_1 = min(1u, (uint(textureSize(arg_0, 0).z) - 1u));
  uint v_2 = (v.inner.tint_builtin_value_0 - 1u);
  uint v_3 = min(uint(1), v_2);
  uvec2 v_4 = (uvec2(textureSize(arg_0, int(v_3)).xy) - uvec2(1u));
  ivec2 v_5 = ivec2(min(uvec2(ivec2(1)), v_4));
  ivec3 v_6 = ivec3(v_5, int(v_1));
  uvec4 res = texelFetch(arg_0, v_6, int(v_3));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec4(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_7ab4df();
  return tint_symbol;
}
void main() {
  VertexOutput v_7 = vertex_main_inner();
  gl_Position = v_7.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_7.prevent_dce;
  gl_PointSize = 1.0f;
}
