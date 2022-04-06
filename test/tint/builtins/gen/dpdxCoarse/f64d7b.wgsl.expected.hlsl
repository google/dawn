void dpdxCoarse_f64d7b() {
  float3 res = ddx_coarse(float3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  dpdxCoarse_f64d7b();
  return;
}
