float3 fwidth_5d1b39() {
  float3 res = fwidth((1.0f).xxx);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store3(0u, asuint(fwidth_5d1b39()));
  return;
}
