void dpdxCoarse_f64d7b() {
  float3 res = ddx_coarse((0.0f).xxx);
}

void fragment_main() {
  dpdxCoarse_f64d7b();
  return;
}
