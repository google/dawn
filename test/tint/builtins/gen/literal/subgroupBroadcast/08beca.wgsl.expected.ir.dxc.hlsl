SKIP: FAILED

float subgroupBroadcast_08beca() {
  float res = WaveReadLaneAt(1.0f, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_08beca();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = subgroupBroadcast_08beca();
  ^

