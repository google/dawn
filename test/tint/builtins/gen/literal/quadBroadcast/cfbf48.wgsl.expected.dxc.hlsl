//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 quadBroadcast_cfbf48() {
  float2 res = QuadReadLaneAt((1.0f).xx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_cfbf48()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 quadBroadcast_cfbf48() {
  float2 res = QuadReadLaneAt((1.0f).xx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_cfbf48()));
}

