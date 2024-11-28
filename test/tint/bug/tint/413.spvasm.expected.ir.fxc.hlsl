
Texture2D<uint4> Src : register(t0);
RWTexture2D<uint4> Dst : register(u1);
void main_1() {
  uint4 srcValue = (0u).xxxx;
  uint3 v = (0u).xxx;
  Src.GetDimensions(0u, v.x, v.y, v.z);
  uint v_1 = min(uint(int(0)), (v.z - 1u));
  uint3 v_2 = (0u).xxx;
  Src.GetDimensions(uint(v_1), v_2.x, v_2.y, v_2.z);
  uint2 v_3 = (v_2.xy - (1u).xx);
  int2 v_4 = int2(min(uint2((int(0)).xx), v_3));
  srcValue = uint4(Src.Load(int3(v_4, int(v_1))));
  srcValue.x = (srcValue.x + 1u);
  uint4 x_27 = srcValue;
  Dst[(int(0)).xx] = x_27;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

