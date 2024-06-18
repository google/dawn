SKIP: FAILED

float3 dpdxFine_f92fb6() {
  float3 res = ddx_fine((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_f92fb6();
}

