void dpdxFine_8c5069() {
  float4 res = ddx_fine((0.0f).xxxx);
}

void fragment_main() {
  dpdxFine_8c5069();
  return;
}
