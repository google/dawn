Texture2D<uint4> Src : register(t0, space0);
RWTexture2D<uint4> Dst : register(u1, space0);

[numthreads(1, 1, 1)]
void main() {
  uint4 srcValue = uint4(0u, 0u, 0u, 0u);
  const uint4 x_18 = Src.Load(int3(0, 0, 0));
  srcValue = x_18;
  const uint x_22 = srcValue.x;
  srcValue.x = (x_22 + asuint(1));
  const uint4 x_27 = srcValue;
  Dst[int2(0, 0)] = x_27;
  return;
}

