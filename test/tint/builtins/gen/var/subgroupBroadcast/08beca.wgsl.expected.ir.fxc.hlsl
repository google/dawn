SKIP: FAILED

float subgroupBroadcast_08beca() {
  float arg_0 = 1.0f;
  float res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_08beca();
}

