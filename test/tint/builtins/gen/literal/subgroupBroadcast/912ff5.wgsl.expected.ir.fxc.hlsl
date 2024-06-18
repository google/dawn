SKIP: FAILED

float3 subgroupBroadcast_912ff5() {
  float3 res = WaveReadLaneAt((1.0f).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_912ff5();
}

