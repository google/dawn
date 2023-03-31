Texture2D<uint4> Src : register(t0);
RWTexture2D<uint4> Dst : register(u1);

void main_1() {
  uint4 srcValue = uint4(0u, 0u, 0u, 0u);
  const uint4 x_18 = Src.Load(int3(0, 0, 0));
  srcValue = x_18;
  const uint x_22 = srcValue.x;
  srcValue.x = (x_22 + 1u);
  const uint4 x_27 = srcValue;
  Dst[(0).xx] = x_27;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
