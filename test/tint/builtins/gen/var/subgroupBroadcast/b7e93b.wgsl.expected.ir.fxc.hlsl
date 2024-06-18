SKIP: FAILED

float4 subgroupBroadcast_b7e93b() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_b7e93b();
}

