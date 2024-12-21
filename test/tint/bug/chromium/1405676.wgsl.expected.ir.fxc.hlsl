
Texture1D<int4> arg_0 : register(t0);
void d() {
  int v = int(int(1));
  int4 v_1 = int4(arg_0.Load(int2(v, int(int(0)))));
  float l = 0.14112000167369842529f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

