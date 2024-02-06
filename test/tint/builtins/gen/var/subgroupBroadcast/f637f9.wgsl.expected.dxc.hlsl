RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_f637f9() {
  int4 arg_0 = (1).xxxx;
  int4 res = WaveReadLaneAt(arg_0, 1u);
  prevent_dce.Store4(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_f637f9();
  return;
}
