void dpdxFine_9631de() {
  float2 res = ddx_fine((1.0f).xx);
}

void fragment_main() {
  dpdxFine_9631de();
  return;
}
