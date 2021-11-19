#version 310 es
precision mediump float;

uniform highp usampler2D Src;
uniform highp writeonly uimage2D Dst;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  uvec4 srcValue = uvec4(0u, 0u, 0u, 0u);
  uvec4 x_22 = texelFetch(Src, ivec2(0, 0), 0);
  srcValue = x_22;
  uint x_24 = srcValue.x;
  uint x_25 = (x_24 + 1u);
  imageStore(Dst, ivec2(0, 0), srcValue.xxxx);
  return;
}
void main() {
  tint_symbol();
}


