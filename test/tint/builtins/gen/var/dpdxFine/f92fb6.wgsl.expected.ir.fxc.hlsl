SKIP: FAILED

float3 dpdxFine_f92fb6() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddx_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_f92fb6();
}

