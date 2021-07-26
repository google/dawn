bug/tint/453.wgsl:7:26 warning: use of deprecated intrinsic
  let x_22 : vec4<u32> = textureLoad(Src, vec2<i32>(0, 0));
                         ^^^^^^^^^^^

Texture2D<uint4> Src : register(t0, space0);
RWTexture2D<uint4> Dst : register(u1, space0);

[numthreads(1, 1, 1)]
void main() {
  uint4 srcValue = uint4(0u, 0u, 0u, 0u);
  const uint4 x_22 = Src.Load(int3(0, 0, 0));
  srcValue = x_22;
  const uint x_24 = srcValue.x;
  const uint x_25 = (x_24 + 1u);
  Dst[int2(0, 0)] = srcValue.xxxx;
  return;
}
