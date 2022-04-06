void dpdxFine_f92fb6() {
  float3 res = ddx_fine(float3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  dpdxFine_f92fb6();
  return;
}
