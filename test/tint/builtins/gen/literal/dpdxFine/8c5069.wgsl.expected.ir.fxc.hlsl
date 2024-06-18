SKIP: FAILED

float4 dpdxFine_8c5069() {
  float4 res = ddx_fine((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_8c5069();
}

