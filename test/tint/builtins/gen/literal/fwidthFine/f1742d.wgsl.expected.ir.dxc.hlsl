
RWByteAddressBuffer prevent_dce : register(u0);
float fwidthFine_f1742d() {
  float v = ddx_fine(1.0f);
  float v_1 = ddy_fine(1.0f);
  float v_2 = abs(v);
  float res = (v_2 + abs(v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(fwidthFine_f1742d()));
}

