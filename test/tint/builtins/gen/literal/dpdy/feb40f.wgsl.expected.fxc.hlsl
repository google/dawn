RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdy_feb40f() {
  float3 res = ddy((1.0f).xxx);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  dpdy_feb40f();
  return;
}
