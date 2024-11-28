#version 310 es
precision highp float;
precision highp int;


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp usampler3D arg_0;
uvec3 textureDimensions_6e6c7a() {
  uvec3 res = uvec3(textureSize(arg_0, int(min(1u, (v_1.inner.tint_builtin_value_0 - 1u)))));
  return res;
}
void main() {
  v.inner = textureDimensions_6e6c7a();
}
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp usampler3D arg_0;
uvec3 textureDimensions_6e6c7a() {
  uvec3 res = uvec3(textureSize(arg_0, int(min(1u, (v_1.inner.tint_builtin_value_0 - 1u)))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_6e6c7a();
}
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v;
uniform highp usampler3D arg_0;
layout(location = 0) flat out uvec3 vertex_main_loc0_Output;
uvec3 textureDimensions_6e6c7a() {
  uvec3 res = uvec3(textureSize(arg_0, int(min(1u, (v.inner.tint_builtin_value_0 - 1u)))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec3(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureDimensions_6e6c7a();
  return tint_symbol;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
