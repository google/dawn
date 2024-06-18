SKIP: FAILED

uint2 subgroupBroadcast_4a4334() {
  uint2 res = WaveReadLaneAt((1u).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_4a4334();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = subgroupBroadcast_4a4334();
  ^

