RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyFine_6eb673() {
  float res = ddy_fine(1.0f);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdyFine_6eb673();
  return;
}
