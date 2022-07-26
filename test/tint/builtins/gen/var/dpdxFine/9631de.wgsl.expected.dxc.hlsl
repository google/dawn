void dpdxFine_9631de() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddx_fine(arg_0);
}

void fragment_main() {
  dpdxFine_9631de();
  return;
}
