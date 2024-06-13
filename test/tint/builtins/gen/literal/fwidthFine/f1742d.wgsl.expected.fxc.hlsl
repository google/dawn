float tint_fwidth_fine(float v) {
  return (abs(ddx_fine(v)) + abs(ddy_fine(v)));
}

float fwidthFine_f1742d() {
  float res = tint_fwidth_fine(1.0f);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store(0u, asuint(fwidthFine_f1742d()));
  return;
}
