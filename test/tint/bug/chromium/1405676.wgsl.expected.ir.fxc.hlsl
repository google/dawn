
Texture1D<int4> arg_0 : register(t0);
void d() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint v_1 = min(uint(int(0)), (v.y - 1u));
  uint2 v_2 = (0u).xx;
  arg_0.GetDimensions(uint(v_1), v_2.x, v_2.y);
  uint v_3 = (v_2.x - 1u);
  int v_4 = int(min(uint(int(1)), v_3));
  int4 v_5 = int4(arg_0.Load(int2(v_4, int(v_1))));
  float l = 0.14112000167369842529f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

