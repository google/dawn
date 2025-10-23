
ByteAddressBuffer G : register(t0);
[numthreads(1, 1, 1)]
void n() {
  uint v = 0u;
  G.GetDimensions(v);
}

