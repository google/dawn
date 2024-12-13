//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, std140)
uniform f_tint_symbol_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp usampler2D arg_0;
uvec4 textureLoad_bc3201() {
  uint v_2 = min(1u, (v_1.inner.tint_builtin_value_0 - 1u));
  ivec2 v_3 = ivec2(uvec2(min(1u, (uvec2(textureSize(arg_0, int(v_2))).x - 1u)), 0u));
  uvec4 res = texelFetch(arg_0, v_3, int(v_2));
  return res;
}
void main() {
  v.inner = textureLoad_bc3201();
}
//
// compute_main
//
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp usampler2D arg_0;
uvec4 textureLoad_bc3201() {
  uint v_2 = min(1u, (v_1.inner.tint_builtin_value_0 - 1u));
  ivec2 v_3 = ivec2(uvec2(min(1u, (uvec2(textureSize(arg_0, int(v_2))).x - 1u)), 0u));
  uvec4 res = texelFetch(arg_0, v_3, int(v_2));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_bc3201();
}
//
// vertex_main
//
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(binding = 0, std140)
uniform v_tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v;
uniform highp usampler2D arg_0;
layout(location = 0) flat out uvec4 tint_interstage_location0;
uvec4 textureLoad_bc3201() {
  uint v_1 = min(1u, (v.inner.tint_builtin_value_0 - 1u));
  ivec2 v_2 = ivec2(uvec2(min(1u, (uvec2(textureSize(arg_0, int(v_1))).x - 1u)), 0u));
  uvec4 res = texelFetch(arg_0, v_2, int(v_1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec4(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_bc3201();
  return tint_symbol;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = v_3.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_interstage_location0 = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
