float dpdyFine_6eb673() {
  float res = ddy_fine(1.0f);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store(0u, asuint(dpdyFine_6eb673()));
  return;
}
