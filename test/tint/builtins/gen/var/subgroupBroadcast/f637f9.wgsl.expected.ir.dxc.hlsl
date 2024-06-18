SKIP: FAILED

int4 subgroupBroadcast_f637f9() {
  int4 arg_0 = (1).xxxx;
  int4 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_f637f9();
}

DXC validation failure:
hlsl.hlsl:9:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = subgroupBroadcast_f637f9();
  ^

