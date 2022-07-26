void dpdxFine_f92fb6() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddx_fine(arg_0);
}

void fragment_main() {
  dpdxFine_f92fb6();
  return;
}
