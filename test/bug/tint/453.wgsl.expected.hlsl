bug/tint/453.wgsl:1:79 warning: use of deprecated language feature: access control is expected as last parameter of storage textures
[[group(0), binding(0)]] var Src : [[access(read)]] texture_storage_2d<r32uint>;
                                                                              ^

bug/tint/453.wgsl:2:80 warning: use of deprecated language feature: access control is expected as last parameter of storage textures
[[group(0), binding(1)]] var Dst : [[access(write)]] texture_storage_2d<r32uint>;
                                                                               ^

Texture2D<uint4> Src : register(t0, space0);
RWTexture2D<uint4> Dst : register(u1, space0);

[numthreads(1, 1, 1)]
void main() {
  uint4 srcValue = uint4(0u, 0u, 0u, 0u);
  const uint4 x_22 = Src.Load(int3(0, 0, 0));
  srcValue = x_22;
  const uint x_24 = srcValue.x;
  const uint x_25 = (x_24 + 1u);
  const uint4 x_27 = srcValue;
  Dst[int2(0, 0)] = x_27.xxxx;
  return;
}

