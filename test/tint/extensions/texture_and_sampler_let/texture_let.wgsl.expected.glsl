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
  uint v_1 = (v.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_2 = min(uint(0), v_1);
  uvec2 v_3 = (uvec2(textureSize(f_tex, int(v_2))) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(ivec2(1)), v_3));
  vec4 res = texelFetch(f_tex, v_4, int(v_2));
  imageStore(f_store, ivec2(0), res);
}
