
Texture2D<uint4> Src : register(t0);
RWTexture2D<uint4> Dst : register(u1);
void main_1() {
  uint4 srcValue = (0u).xxxx;
  int2 v = int2((int(0)).xx);
  srcValue = uint4(Src.Load(int3(v, int(int(0)))));
  srcValue[0u] = (srcValue.x + 1u);
  uint4 x_27 = srcValue;
  Dst[(int(0)).xx] = x_27;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

