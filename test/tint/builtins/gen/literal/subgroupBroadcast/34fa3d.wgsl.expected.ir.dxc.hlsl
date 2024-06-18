SKIP: FAILED

uint3 subgroupBroadcast_34fa3d() {
  uint3 res = WaveReadLaneAt((1u).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_34fa3d();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = subgroupBroadcast_34fa3d();
  ^

