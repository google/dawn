#version 310 es

layout(binding = 1, r32ui) uniform highp writeonly uimage2D Dst;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v;
uniform highp usampler2D Src;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 srcValue = uvec4(0u);
  uint v_1 = min(0u, (v.metadata[0u].x - 1u));
  ivec2 v_2 = ivec2(min(uvec2(0u), (uvec2(textureSize(Src, int(v_1))) - uvec2(1u))));
  uvec4 x_22 = texelFetch(Src, v_2, int(v_1));
  srcValue = x_22;
  uint x_24 = srcValue.x;
  uint x_25 = (x_24 + 1u);
  uvec4 x_27 = srcValue;
  imageStore(Dst, ivec2(0), x_27.xxxx);
}
