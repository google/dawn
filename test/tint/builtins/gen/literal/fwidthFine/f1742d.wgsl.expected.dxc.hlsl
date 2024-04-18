float tint_fwidth_fine(float v) {
  return (abs(ddx_fine(v)) + abs(ddy_fine(v)));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidthFine_f1742d() {
  float res = tint_fwidth_fine(1.0f);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  fwidthFine_f1742d();
  return;
}
