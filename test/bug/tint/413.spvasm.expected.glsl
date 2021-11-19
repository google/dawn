#version 310 es
precision mediump float;

uniform highp usampler2D Src;
uniform highp writeonly uimage2D Dst;

void main_1() {
  uvec4 srcValue = uvec4(0u, 0u, 0u, 0u);
  uvec4 x_18 = texelFetch(Src, ivec2(0, 0), 0);
  srcValue = x_18;
  uint x_22 = srcValue.x;
  srcValue.x = (x_22 + uint(1));
  imageStore(Dst, ivec2(0, 0), srcValue);
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


