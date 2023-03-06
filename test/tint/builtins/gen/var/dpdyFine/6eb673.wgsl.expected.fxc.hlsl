RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyFine_6eb673() {
  float arg_0 = 1.0f;
  float res = ddy_fine(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  dpdyFine_6eb673();
  return;
}
