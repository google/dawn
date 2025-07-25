//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 quadBroadcast_b68331() {
  uint4 res = QuadReadLaneAt((1u).xxxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, quadBroadcast_b68331());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 quadBroadcast_b68331() {
  uint4 res = QuadReadLaneAt((1u).xxxx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, quadBroadcast_b68331());
}

