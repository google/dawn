
RWByteAddressBuffer prevent_dce : register(u0);
float fwidthFine_f1742d() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(fwidthFine_f1742d()));
}

