#version 310 es
precision highp float;
precision highp int;

layout(binding = 1, r32f) uniform highp image2D f_store;
layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2D f_tex;
void main() {
  uint v_1 = min(0u, (v.metadata[0u].x - 1u));
  ivec2 v_2 = ivec2(min(uvec2(1u), (uvec2(textureSize(f_tex, int(v_1))) - uvec2(1u))));
  vec4 res = texelFetch(f_tex, v_2, int(v_1));
  imageStore(f_store, ivec2(0), res);
}
