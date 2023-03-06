RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidthCoarse_159c8a() {
  float arg_0 = 1.0f;
  float res = fwidth(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  fwidthCoarse_159c8a();
  return;
}
