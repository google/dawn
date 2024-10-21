#version 310 es

layout(binding = 1, r32ui) uniform highp writeonly uimage2D Dst;
uniform highp usampler2D Src;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 srcValue = uvec4(0u);
  ivec2 v = ivec2(ivec2(0));
  uvec4 x_22 = texelFetch(Src, v, int(0));
  srcValue = x_22;
  uint x_24 = srcValue.x;
  uint x_25 = (x_24 + 1u);
  uvec4 x_27 = srcValue;
  imageStore(Dst, ivec2(0), x_27.xxxx);
}
