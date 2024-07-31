
Texture2D<uint4> Src : register(t0);
RWTexture2D<uint4> Dst : register(u1);
void main_1() {
  uint4 srcValue = (0u).xxxx;
  Texture2D<uint4> v = Src;
  int2 v_1 = int2((0).xx);
  srcValue = uint4(v.Load(int3(v_1, int(0))));
  srcValue[0u] = (srcValue.x + 1u);
  uint4 x_27 = srcValue;
  Dst[(0).xx] = x_27;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

