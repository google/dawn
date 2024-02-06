RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_5196c8() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WaveReadLaneAt(arg_0, 1u);
  prevent_dce.Store2(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_5196c8();
  return;
}
