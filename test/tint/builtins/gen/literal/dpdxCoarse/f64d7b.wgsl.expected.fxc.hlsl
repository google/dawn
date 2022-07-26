void dpdxCoarse_f64d7b() {
  float3 res = ddx_coarse((1.0f).xxx);
}

void fragment_main() {
  dpdxCoarse_f64d7b();
  return;
}
