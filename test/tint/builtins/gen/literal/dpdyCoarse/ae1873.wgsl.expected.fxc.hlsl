void dpdyCoarse_ae1873() {
  float3 res = ddy_coarse((1.0f).xxx);
}

void fragment_main() {
  dpdyCoarse_ae1873();
  return;
}
