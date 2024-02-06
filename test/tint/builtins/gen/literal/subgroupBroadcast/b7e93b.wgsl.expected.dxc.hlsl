RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_b7e93b() {
  float4 res = WaveReadLaneAt((1.0f).xxxx, 1u);
  prevent_dce.Store4(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_b7e93b();
  return;
}
