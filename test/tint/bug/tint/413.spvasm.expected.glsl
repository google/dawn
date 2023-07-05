#version 310 es

layout(r32ui) uniform highp writeonly uimage2D Dst;
uniform highp usampler2D Src_1;
void main_1() {
  uvec4 srcValue = uvec4(0u, 0u, 0u, 0u);
  srcValue = texelFetch(Src_1, ivec2(0), 0);
  srcValue.x = (srcValue.x + 1u);
  uvec4 x_27 = srcValue;
  imageStore(Dst, ivec2(0), x_27);
  return;
}

void tint_symbol() {
  main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
