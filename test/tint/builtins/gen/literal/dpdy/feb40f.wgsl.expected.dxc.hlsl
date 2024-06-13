float3 dpdy_feb40f() {
  float3 res = ddy((1.0f).xxx);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store3(0u, asuint(dpdy_feb40f()));
  return;
}
