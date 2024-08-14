
RWByteAddressBuffer prevent_dce : register(u0);
uint3 quadBroadcast_f1e8ec() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = QuadReadLaneAt(arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, quadBroadcast_f1e8ec());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, quadBroadcast_f1e8ec());
}

