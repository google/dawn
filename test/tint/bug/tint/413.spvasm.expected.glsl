#version 310 es

layout(binding = 1, r32ui) uniform highp writeonly uimage2D Dst;
uniform highp usampler2D Src;
void main_1() {
  uvec4 srcValue = uvec4(0u);
  ivec2 v = ivec2(ivec2(0));
  srcValue = texelFetch(Src, v, int(0));
  srcValue[0u] = (srcValue.x + 1u);
  uvec4 x_27 = srcValue;
  imageStore(Dst, ivec2(0), x_27);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
