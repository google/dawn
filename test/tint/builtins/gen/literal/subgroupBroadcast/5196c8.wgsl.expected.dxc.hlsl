RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_5196c8() {
  float2 res = WaveReadLaneAt((1.0f).xx, 1u);
  prevent_dce.Store2(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_5196c8();
  return;
}
